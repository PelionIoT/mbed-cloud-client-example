#!/usr/bin/env python

#################################################################################
#  Copyright 2016-2019 ARM Ltd.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#################################################################################

import json
import os
import logging
import sys
import re
import subprocess
import stat
import tarfile
import zipfile
from string import Template
import shutil
import platform
import tempfile
from contextlib import contextmanager
import requests
import click

logger = logging.getLogger('pal-platform')
# logger.level = logging.INFO
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
PAL_PLATFORM_ROOT = SCRIPT_DIR
PROG_NAME = os.path.basename(sys.argv[0])
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])
PATCH_UTIL = 'patch.exe' if platform.system() == 'Windows' else 'patch'
AVAILABLE_TARGETS = []
BUILD_SYS_MIN_VER = 2
PLAT_CMAKE_TEMPLATE = '''
#################################################################################
#                                                                               #
#                        THIS IS AN AUTO GENERATED FILE                         #
#                                                                               #
#################################################################################

set (MBED_CLOUD_CLIENT_SDK $mbed_cloud_client_sdk)
set (MBED_CLOUD_CLIENT_OS $mbed_cloud_client_os)
set (MBED_CLOUD_CLIENT_DEVICE $mbed_cloud_client_device)
set (MBED_CLOUD_CLIENT_MIDDLEWARE $mbed_cloud_client_mw_list)
set (MBED_CLOUD_CLIENT_TOOLCHAIN $mbed_cloud_client_toolchain)
set (MBED_CLOUD_CLIENT_BUILD_SYS_MIN_VER $mbed_cloud_client_build_sys_min_ver)
'''

# for 2.7 compatibility:
# http://stackoverflow.com/questions/1713038/super-fails-with-error-typeerror-argument-1-must-be-type-not-classobj
__metaclass__ = type


class Config(object):
    def __init__(self):
        self.verbose = False


pass_config = click.make_pass_decorator(Config, ensure=True)


class DynamicChoice(click.Choice):
    name = 'DynamicChoice'

    def __init__(self, func, **kwargs):
        self.choices = []
        self.func = func
        self.kwargs = kwargs

    def get_metavar(self, param):
        self.choices = self.func(**self.kwargs)
        return super(DynamicChoice, self).get_metavar(param)

    def get_missing_message(self, param):
        self.choices = self.func(**self.kwargs)
        return super(DynamicChoice, self).get_missing_message(param)

    def convert(self, value, param, ctx):
        self.choices = self.func(**self.kwargs)
        return super(DynamicChoice, self).convert(value, param, ctx)

    def __repr__(self):
        self.choices = self.func(**self.kwargs)
        return super(DynamicChoice, self).__repr__()



@contextmanager
def TemporaryDirectory():
    name = tempfile.mkdtemp()
    try:
        yield name
    finally:
        shutil.rmtree(name, onerror=del_rw)


def del_rw(action, path, exc):
    """
    Callback function for error handling in shutil.rmtree.
    Will be called whenever shutil.rmtree catches an exception.

    :param action: The internal action used inside shutil.rmtree
        (os.listdir, os.remove, or os.rmdir)
    :param path: The path of the file/directory
    :param exc: The Exception info
    """
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        action(path)
    else:
        raise


def is_git_pull_required(repo_dir, branch, **stream_kwargs):
    """
    Check if git pull is required - in case sources are modified,
    pull will fail, so we check whether pull is required before
    failing the script.
    http://stackoverflow.com/questions/3258243/check-if-pull-needed-in-git

    :param repo_dir: Directory to check
    :param branch: Branch name / hash tag
    :param stream_kwargs:
        * *stdout* --
          Standard output handle
        * *stderr* --
          Standard error handle

    :return: True/False
    """
    logger.debug('Check if git pull required for %s', repo_dir)
    active_branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=repo_dir).strip()
    if active_branch.decode(encoding='utf-8') == 'HEAD':
        return False
    local_hash = check_output(['git', 'rev-parse', '@'], cwd=repo_dir, **stream_kwargs)
    remote_hash = check_output(['git', 'rev-parse', '@{upstream}'], cwd=repo_dir, **stream_kwargs)
    base_hash = check_output(['git', 'merge-base', '@', '@{upstream}'], cwd=repo_dir, **stream_kwargs)
    if local_hash == remote_hash:
        logger.info('%s is up-to-date with %s', repo_dir, branch)
        return False
    elif local_hash == base_hash:
        return True
    elif remote_hash == base_hash:
        logger.warning('There are local commits - do not forget to push %s', repo_dir)
        return False
    else:
        raise Exception('Local and remote repositories of %s have diverged', repo_dir)


def is_git_dir(_dir):
    return os.path.isdir(_dir) and os.path.isdir(os.path.join(_dir, '.git'))

def extract_repo_name(url):
    """
    Extract repository remote and name from a git URL, ignoring protocol and .git endings
    """
    regex_git_url = r'^(git\://|ssh\://|https?\://|)(([^/:@]+)(\:([^/:@]+))?@)?([^/:]+)[:/](.+?)(\.git|\/?)$'
    m = re.match(regex_git_url, url)
    return m.group(7) or url


def git_fetch_submodule(submodule_name, overwrite_url, overwrite_branch, dest_dir, **stream_kwargs):
    """
    Synchronize and fetch submodule sources to a local directory

    :param overwrite_url: URL to overwrite submodule URL or None to use default
    :param overwrite_branch: Branch name to overwrite submodule branch/hash or None to use default
    :param dest_dir: Destination directory
    :param stream_kwargs:
        * *stdout* --
          Standard output handle
        * *stderr* --
          Standard error handle
    """
    
    # Overwrite submodule configuration in `.gitmodules`
    if overwrite_url:
        check_cmd(['git', 'config', '--file=.gitmodules', 'submodule.{}.url'.format(submodule_name), overwrite_url], cwd=dest_dir, **stream_kwargs)
    if overwrite_branch:
        check_cmd(['git', 'config', '--file=.gitmodules', 'submodule.{}.branch'.format(submodule_name), overwrite_branch], cwd=dest_dir, **stream_kwargs)

    # Synchronizes submodules' configuration
    check_cmd(['git', 'submodule', 'sync', '--', submodule_name], cwd=dest_dir, **stream_kwargs)

    url = check_output(['git', 'config', '--file=.gitmodules', 'submodule.{}.url'.format(submodule_name)], cwd=dest_dir).decode(encoding='utf-8').strip()
    # Update submodule to match superproject expects
    if overwrite_url or overwrite_branch:
        branch = check_output(['git', 'config', '--file=.gitmodules', 'submodule.{}.branch'.format(submodule_name)], cwd=dest_dir).decode(encoding='utf-8').strip()
        logger.warning('Updating submodule \'%s\' from %s at \'%s\' remote branch HEAD', submodule_name, url, branch)
        # Use submodule configuration to update the submodule
        check_cmd(['git', 'submodule', 'update', '--init', '--recursive', '--remote', '--', submodule_name], cwd=dest_dir, **stream_kwargs)
    else:
        logger.info('Updating submodule \'%s\' from %s at superproject\'s commited hash', submodule_name, url)
        # Use the superproject's recorded SHA-1 to update the submodule
        check_cmd(['git', 'submodule', 'update', '--init', '--recursive', '--', submodule_name], cwd=dest_dir, **stream_kwargs)

def git_fetch(git_url, tree_ref, dest_dir, submodules, **stream_kwargs):
    """
    Fetch sources from a git url to a local directory

    :param git_url: Url of git repository
    :param tree_ref: Branch name / hash tag
    :param dest_dir: Destination directory
    :param submodules: dictionary describe submodules to fetch (may be None)
    :param stream_kwargs:
        * *stdout* --
          Standard output handle
        * *stderr* --
          Standard error handle
    """
    is_hash = re.search('[a-fA-F0-9]{40}', tree_ref)

    if not is_git_dir(dest_dir):
        logger.info('Cloning from %s at %s to %s', git_url, tree_ref, dest_dir)
        cmd = ['git', 'clone'] + (['--no-checkout'] if is_hash else ['-b', tree_ref])
        check_cmd(cmd + [git_url, dest_dir], **stream_kwargs)

        check_cmd(['git', 'config', 'core.longpaths', 'true'], cwd=dest_dir, **stream_kwargs)
        if is_hash:
            check_cmd(['git', 'config', 'advice.detachedHead', 'false'], cwd=dest_dir, **stream_kwargs)
            check_cmd(['git', 'checkout', tree_ref], cwd=dest_dir, **stream_kwargs)
    else:
        logger.info('%s already exists, updating from %s', dest_dir, git_url)
        remote_url = check_output(['git', 'ls-remote', '--get-url'], cwd=dest_dir).decode(encoding='utf-8').strip()
        assert extract_repo_name(git_url) == extract_repo_name(remote_url), 'Trying to update %s from different remote (%s)' % (dest_dir, git_url)

        check_cmd(['git', 'fetch', '--all'], cwd=dest_dir, **stream_kwargs)

        if not is_hash:
            active_branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=dest_dir).strip()
            if active_branch.decode(encoding='utf-8') == tree_ref:
                if is_git_pull_required(dest_dir, tree_ref, **stream_kwargs):
                    check_cmd(['git', 'pull', '--rebase', '--tags', '--all'], cwd=dest_dir, **stream_kwargs)
                else:
                    return

        logger.info('Checking out from %s at %s', git_url, tree_ref)
        check_cmd(['git', 'checkout', tree_ref], cwd=dest_dir, **stream_kwargs)

        if not is_hash and is_git_pull_required(dest_dir, tree_ref, **stream_kwargs):
            check_cmd(['git', 'pull', '--rebase', '--tags', '--all'], cwd=dest_dir, **stream_kwargs)

    if submodules:
        # Reset .gitmodules to remove previously synced changes 
        check_cmd(['git', 'checkout', '.gitmodules'], cwd=dest_dir, **stream_kwargs)
        for submodule_name, submodule_desc in submodules.items():
            git_fetch_submodule(submodule_name, 
                               submodule_desc.get('overwrite-url', None),
                               submodule_desc.get('overwrite-branch', None), 
                               dest_dir, **stream_kwargs)


def download_file(url, dest_dir, file_name=None):
    """
    Download a file from a url to a local directory

    :param url: Url of the remote file
    :param dest_dir: Destination directory
    :param file_name: Local file name (Optional, if missing than a temporary name is given)

    :return: Full path of the downloaded file
    """
    assert (os.path.isdir(dest_dir)), '%s does not exist or not a directory' % dest_dir
    r = requests.get(url, stream=True)
    r.raise_for_status()

    try:
        if file_name:
            fh = open(os.path.join(dest_dir, file_name), 'wb')
        else:
            fh = tempfile.NamedTemporaryFile(dir=dest_dir, delete=False)
        name = fh.name
        shutil.copyfileobj(r.raw, fh)
    finally:
        fh.close()

    return name


def extract_file(path, to_directory):
    """
    Extract an archive file (ZIP/TAR) to a local directory

    :param path: Full path of the archive
    :param to_directory: Destination directory
    """
    if zipfile.is_zipfile(path):
        archive_type, opener = 'ZIP', zipfile.ZipFile
    elif tarfile.is_tarfile(path):
        archive_type, opener = 'TAR', tarfile.open
    else:
        raise Exception('%s has unknown archive type' % path)

    with opener(path) as archive:
        if archive_type == 'ZIP':
            root = archive.infolist()[0]
            root_dir = root.filename
            nested = root.filename.endswith('/')
        elif archive_type == 'TAR':
            root = archive.getmembers()[0]
            root_dir = root.name
            nested = root.isdir()

        if nested:
            with TemporaryDirectory() as temp_dir:
                archive.extractall(path=temp_dir)
                if os.path.isdir(to_directory):
                    shutil.rmtree(to_directory, onerror=del_rw)
                shutil.copytree(os.path.join(temp_dir, root_dir), to_directory)
        else:
            archive.extractall(path=to_directory)


def apply_patch(patch_file, reverse=False, **stream_kwargs):
    """
    Apply patch on a directory

    :param patch_file: Patch file to apply
    :param reverse: True if un-applying an already patched directory
    :param stream_kwargs:
        * *stdout* --
          Standard output handle
        * *stderr* --
          Standard error handle

    :return: True if the directory is already integrated, otherwise False
    """
    logger.info('%s %s', 'Reverting' if reverse else 'Applying', patch_file)
    _dir, filename = os.path.split(patch_file)

    with open(patch_file, 'rt') as fh:
        patch_source = fh.read()
    match = re.search(r'^--- (\S+)', patch_source, re.MULTILINE)
    if not match:
        raise Exception('malformed patch file')

    path_list = match.group(1).split('/')
    strip_num = path_list.index(os.path.splitext(filename)[0])
    logger.debug('patch file relative strip is %s', strip_num)

    cmd = [PATCH_UTIL, '-p', str(strip_num), '-i', patch_file, '--binary']
    cmd += ['--verbose'] if logger.isEnabledFor(logging.DEBUG) else ['--quiet']

    is_integrated = False
    try:
        check_cmd_and_raise(cmd + ['--reverse', '--dry-run', '--force'], cwd=_dir, **stream_kwargs)
        is_integrated = True
        logger.info('%s already integrated, %s', patch_file, 'reverting' if reverse else 'no need to patch')
    except subprocess.CalledProcessError:
        pass
    else:  # exception was not raised
        if reverse:
            check_cmd(cmd + ['--reverse', '--force'], cwd=_dir, **stream_kwargs)
            is_integrated = False
            logger.info('Successfully un-applied %s to %s', patch_file, _dir)

    if not is_integrated and not reverse:
        try:
            check_cmd_and_raise(cmd + ['--dry-run'], cwd=_dir, **stream_kwargs)
            check_cmd_and_raise(cmd, cwd=_dir, **stream_kwargs)
        except subprocess.CalledProcessError:
            click.echo('Applying %s on %s failed, check target directory is clean' % (patch_file, _dir))
            raise click.Abort
        logger.info('Successfully applied %s to %s', patch_file, _dir)
    return is_integrated


def generate_plat_cmake(target):
    """
    Generate target-dependent cmake files

    :param target: The target to generate files to
    :return: Full path of target-dependent output directory
    """
    _os = target.os.name
    if target.os.version:
        _os += '_' + target.os.version
    _device = target.device.name
    _mw_list = [mw.name for mw in target.middleware]
    _sdk = ' '

    if target.name == 'K64F_FreeRTOS_mbedtls':
        _os, _device = (' ', ' ')
        _mw_list = []
        _sdk = 'K64F_FreeRTOS'

    out_dir_name = '__' + target.name
    parent_dir = os.path.normpath(os.path.join(PAL_PLATFORM_ROOT, os.pardir))
    out_dir = os.path.join(parent_dir, out_dir_name)
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir, onerror=del_rw)
    os.makedirs(out_dir)

    autogen_file = os.path.join(out_dir, 'autogen.cmake')
    cmake_template = Template(PLAT_CMAKE_TEMPLATE)
    with open(autogen_file, 'wt') as fh:
        fh.write(
            cmake_template.safe_substitute(
                mbed_cloud_client_sdk=_sdk,
                mbed_cloud_client_os=_os,
                mbed_cloud_client_device=_device,
                mbed_cloud_client_mw_list=' '.join(_mw_list),
                mbed_cloud_client_toolchain=' ',
                mbed_cloud_client_build_sys_min_ver=BUILD_SYS_MIN_VER,
            )
        )
    logger.info('Generated %s', autogen_file)
    parent_cmake = os.path.join(parent_dir, 'CMakeLists.txt')
    if not os.path.isfile(os.path.join(parent_cmake)):
        with open(parent_cmake, 'wt') as fh:
            fh.write('ADDSUBDIRS()\n')
        logger.info('Generated %s', parent_cmake)
    return out_dir


def check_cmd_and_raise(cmd, **kwargs):
    """
    Wrapper function for subprocess.check_call

    :param cmd: The command to execute
    :param kwargs: See `https://docs.python.org/2/library/subprocess.html#subprocess.Popen`_
    """
    logger.debug(" ".join(cmd))

    subprocess.check_call(cmd, **kwargs)
    
def check_cmd(cmd, **kwargs):
    """
    Wrapper function for subprocess.check_call

    :param cmd: The command to execute
    :param kwargs: See `https://docs.python.org/2/library/subprocess.html#subprocess.Popen`_
    """
    logger.debug(" ".join(cmd))
    try:
        subprocess.check_call(cmd, **kwargs)
    except Exception as e:
        logger.error(e)
        logger.error("** failed to run command  %s **", cmd)
        sys.exit()


def check_output(cmd, **kwargs):
    """
    Wrapper function for subprocess.check_output

    :param cmd: The command to execute
    :param kwargs: See `https://docs.python.org/2/library/subprocess.html#subprocess.Popen`_
    :return: Output of subprocess.check_output
    """
    kwargs.pop('stdout', None)
    logger.debug(" ".join(cmd))
    try:
        output = subprocess.check_output(cmd, **kwargs)
    except Exception as e:
        logger.error(e)
        logger.error("** failed to run command  %s **", cmd)
        sys.exit()
    return output

def check_output_and_raise(cmd, **kwargs):
    """
    Wrapper function for subprocess.check_output

    :param cmd: The command to execute
    :param kwargs: See `https://docs.python.org/2/library/subprocess.html#subprocess.Popen`_
    :return: Output of subprocess.check_output
    """
    kwargs.pop('stdout', None)
    logger.debug(" ".join(cmd))
    return subprocess.check_output(cmd, **kwargs)

class Source:
    def __init__(self, src, stream_kwargs):
        self.location = src['location']
        self.stream_kwargs = stream_kwargs

class GitSource(Source):
    def __init__(self, src, stream_kwargs):
        super(GitSource, self).__init__(src, stream_kwargs)
        self.tag = src.get('tag', 'master')
        self.submodules = src.get('submodules', None)

    def write(self, dst, out=sys.stdout):
        out.write('- Clone %s at %s to %s' % (self.location, self.tag, dst))

    def fetch(self, dst, name):
        logger.info('Getting %s from git', name)
        try:
            git_fetch(self.location, self.tag, dst, self.submodules, **self.stream_kwargs)
        except Exception as e:
            logger.error(e)
            logger.error("** failed to fetch %s from git - please check that remote is correct and avialable **", name)
            sys.exit()

class LocalSource(Source):
    def __init__(self, src, stream_kwargs):
        super(LocalSource, self).__init__(src, stream_kwargs)

    def write(self, dst, out=sys.stdout):
        out.write('- Copy %s to %s' % (self.location, dst))

    def fetch(self, dst, name):
        assert os.path.isdir(self.location)
        logger.info('Copying %s from local folder %s to %s', name, self.location, dst)
        if os.path.isdir(dst):
            logger.warning('%s already exists, overriding it..', dst)
            shutil.rmtree(dst, onerror=del_rw)
        shutil.copytree(self.location, dst)


class RemoteArchiveSource(Source):
    def __init__(self, src, stream_kwargs):
        super(RemoteArchiveSource, self).__init__(src, stream_kwargs)

    def write(self, dst, out=sys.stdout):
        out.write('- Download %s and extract to %s' % (self.location, dst))

    def fetch(self, dst, name):
        with TemporaryDirectory() as temp_dir:
            logger.info('Downloading %s from %s to %s', name, self.location, temp_dir)
            _file = download_file(self.location, temp_dir)
            logger.info('Extracting %s to %s', _file, dst)
            extract_file(_file, dst)


class RemoteFilesSource(Source):
    def __init__(self, src, stream_kwargs):
        super(RemoteFilesSource, self).__init__(src, stream_kwargs)

    def write(self, dst, out=sys.stdout):
        for location in self.location:
            out.write('- Download %s to %s' % (location, dst))

    def fetch(self, dst, name):
        if not os.path.isdir(dst):
            os.makedirs(dst)
        logger.info('Getting %s files', name)
        for location in self.location:
            logger.info('Downloading %s to %s', location, dst)
            download_file(location, dst, location.split('/')[-1])


class SourceFactory(object):
    @staticmethod
    def get_source(src, stream_kwargs):
        sources = {
            'git': GitSource,
            'local': LocalSource,
            'remote-archive': RemoteArchiveSource,
            'remote-files': RemoteFilesSource
        }
        protocol = src['protocol']
        assert protocol in list(sources.keys()), \
            '%s is not a valid protocol, valid protocols are %s' % (protocol, list(sources.keys()))
        return sources[protocol](src, stream_kwargs)


class Element:
    def __init__(self, data, stream_kwargs, name=None):
        self.name = data.get('name', name)
        self.version = data.get('version', None)
        self.comment = data.get('comment', None)
        patch_file = data.get('patch_file', None)
        if patch_file:
            patch_file = os.path.join(PAL_PLATFORM_ROOT, patch_file)
            patch_file = patch_file.replace("/", "\\") if platform.system() == 'Windows' else patch_file
        self.patch_file = patch_file
        self.source = SourceFactory.get_source(data['from'], stream_kwargs) if 'from' in data else None
        dst = data.get('to', None)
        if dst:
            assert self.source, 'missing "from" field in %s' % self.name
            dst = os.path.join(PAL_PLATFORM_ROOT, dst)
            dst = dst.replace("/", "\\") if platform.system() == 'Windows' else dst
        else:
            assert not self.source, 'missing "to" field in %s' % self.name
        self.destination = dst
        self.stream_kwargs = stream_kwargs

    def is_fetch_needed(self):
        return self.destination and not os.path.isdir(self.destination)

    def write(self, out=sys.stdout):
        if self.source:
            out.write('\n')
            name = self.name if self.name else ''
            version = self.version if self.version else ''
            out.write('%s\n' % ('#' * (len(name) + len(version) + 5)))
            out.write('# %s %s\n' % (name, version))
            out.write('%s\n' % ('#' * (len(name) + len(version) + 5)))
            if self.comment:
                out.write(self.comment + '\n\n')
            self.source.write(self.destination, out)
            if self.patch_file:
                out.write('- Apply patch %s\n' % self.patch_file)

    def fetch(self):
        if self.source:
            self.source.fetch(self.destination, self.name)

    def delete(self):
        if self.destination and os.path.isdir(self.destination):
            logger.info('Deleting %s', self.destination)
            shutil.rmtree(self.destination, onerror=del_rw)

    def apply_patch(self):
        if self.patch_file:
            is_integrated = apply_patch(self.patch_file, **self.stream_kwargs)
            if not is_integrated and self.source.__class__.__name__ == 'GitSource':
                check_cmd(['git', 'config', 'user.name', 'pal-platform'], cwd=self.destination, **self.stream_kwargs)
                check_cmd(['git', 'config', 'user.email', '<>'], cwd=self.destination, **self.stream_kwargs)
                check_cmd(['git', 'add', '--all'], cwd=self.destination, **self.stream_kwargs)
                message = 'applied patch: %s' % self.patch_file
                check_cmd(['git', 'commit', '-m', message], cwd=self.destination, **self.stream_kwargs)


class Target(Element):
    """
    Class which describes an mbed-cloud-client supported target and it's operations.
    For supported targets run::

        pal-platform deploy -h

    Target's operations are

    * Writing deployment instructions to stdout/file.

    * Fetching the required target-dependent sources according to pal-platform.json.

    * Deleting fetched target-dependent sources according to pal-platform.json.

    * Apply patches if needed on the deployed sources according to pal-platform.json.

    :param name: Target name
    :param data: Dictionary describing the target (read from pal-platform.json)
    :param stream_kwargs:
        * *stdout* --
          Standard output handle
        * *stderr* --
          Standard error handle
    """

    def __init__(self, name, data, stream_kwargs):
        super(Target, self).__init__(data, stream_kwargs, name)
        self.os = Element(data['os'], stream_kwargs)
        self.device = Element(data['device'], stream_kwargs)
        mw = data.get('middleware', {})
        self.middleware = []
        for k in mw:
            self.middleware.append(Element(mw[k], stream_kwargs, k))

    def is_fetch_needed(self):
        fetch_needed = super(Target, self).is_fetch_needed() or \
                       self.os.is_fetch_needed() or \
                       self.device.is_fetch_needed()
        for mw in self.middleware:
            fetch_needed = fetch_needed or mw.is_fetch_needed()
        return fetch_needed

    def write_elements(self, out):
        """
        Write instructions on how to deploy the elements of the target

        :param out: Where to write the instructions to (stdout / file)
        """
        out.write('%s %s %s\n' % ('~' * 30, self.name, '~' * (80 - (22 + len(self.name)))))
        self.write(out)
        self.os.write(out)
        self.device.write(out)
        for mw in self.middleware:
            mw.write(out)
        out.write('\n')

    def fetch_elements(self):
        """
        Fetch the required target-dependent sources for the target according to pal-platform.json
        """
        self.fetch()
        self.os.fetch()
        self.device.fetch()
        for mw in self.middleware:
            mw.fetch()

    def delete_elements(self):
        """
        Delete the required target-dependent sources of the target according to pal-platform.json
        """
        self.os.delete()
        self.device.delete()
        for mw in self.middleware:
            mw.delete()
        self.delete()

    def patch_elements(self):
        """
        Apply patches if needed on the deployed sources of the target according to pal-platform.json
        """
        self.apply_patch()
        self.os.apply_patch()
        self.device.apply_patch()
        for mw in self.middleware:
            mw.apply_patch()


def json_read(file_name):
    """
    Helper function that loads JSON file with all string values and keys represented as string objects

    :param file_name: JSON file to read
    :return: Dictionary representation of the JSON file
    """
    with open(file_name, 'rt') as fh:
        try:
            return json.load(fh)
        except ValueError as config_parse_exception:
            raise Exception(
                'Malformed %s - %s' % (file_name, config_parse_exception)
            )


def get_available_targets():
    return AVAILABLE_TARGETS


def get_available_toolchains():
    return AVAILABLE_TOOLCHAINS


@click.group(context_settings=CONTEXT_SETTINGS, chain=True)
@click.option('-v', '--verbose', is_flag=True, help='Turn ON verbose mode')
@click.option(
    '--from-file',
    type=click.Path(exists=True, file_okay=True, readable=True, resolve_path=True),
    default=os.path.join(SCRIPT_DIR, 'pal-platform.json'),
    help='Path to a .json file containing the supported targets configuration.\n'
         'Default is %s' % os.path.normpath(os.path.join(SCRIPT_DIR, 'pal-platform.json'))
)
@click.version_option(version='1.2')
@pass_config
def cli(config, verbose, from_file):
    config.verbose = verbose
    config.stream_kwargs = {'stdout': open(os.devnull, 'w'), 'stderr': subprocess.STDOUT}
    config.targets = json_read(from_file)
    global AVAILABLE_TARGETS
    AVAILABLE_TARGETS = list(config.targets.keys())

    parent_dir = os.path.normpath(os.path.join(from_file, os.pardir))
    toolchain_dir = os.path.join(parent_dir, "Toolchain")
    toolchain_list = os.listdir(toolchain_dir)
    global AVAILABLE_TOOLCHAINS
    AVAILABLE_TOOLCHAINS = toolchain_list

    logging.basicConfig(
        level=logging.DEBUG if verbose else logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        stream=sys.stdout
    )
    if logger.isEnabledFor(logging.DEBUG):
        config.stream_kwargs = {}


@cli.command(
    context_settings=CONTEXT_SETTINGS,
    short_help='Deploy mbed-cloud-client files (run "%s deploy -h" for help)' % PROG_NAME
)
@click.option(
    '--target',
    'target_name',
    help='The target to deploy platform-dependent files for',
    required=True,
    type=DynamicChoice(get_available_targets)
)
@click.option('--skip-update', is_flag=True, help='Skip Git Repositories update')
@click.option('-i', '--instructions', is_flag=True, help='Show deployment instructions for a given target and exit.')
@pass_config
def deploy(config, target_name, skip_update, instructions):
    """Deploy target-dependent files"""
    config.target_name = target_name
    config.skip_update = skip_update

    target = Target(config.target_name, config.targets[target_name], config.stream_kwargs)

    if instructions:
        target.write_elements(sys.stdout)
        click.get_current_context().exit()

    if not config.skip_update:
        target.fetch_elements()
    target.patch_elements()

    instructions_file = os.path.join(PAL_PLATFORM_ROOT, target.name + '.txt')
    with open(instructions_file, 'wt') as fh:
        target.write_elements(fh)
    click.echo(click.style('Deployment for %s is successful.' % config.target_name, fg='green'))
    click.echo(click.style('Deployment instructions are in %s.' % instructions_file, fg='green'))


@cli.command(
    context_settings=CONTEXT_SETTINGS,
    short_help='Generate platform-dependent files (run "%s generate -h" for help)' % PROG_NAME
)
@click.option(
    '--target',
    'target_name',
    help='The target to generate platform-dependent files for',
    type=DynamicChoice(get_available_targets)
)
@pass_config
def generate(config, target_name):
    """Generate files to be used by build-system"""
    if target_name:
        config.target_name = target_name
    if not hasattr(config, 'target_name'):
        ctx = click.get_current_context()
        raise click.MissingParameter(ctx=ctx, param=ctx.command.params[0])
    else:
        target = Target(config.target_name, config.targets[config.target_name], config.stream_kwargs)
        if target.is_fetch_needed():
            click.echo(
                'Target %s is not deployed, please run "%s deploy --target %s" first.' %
                (config.target_name, PROG_NAME, config.target_name)
            )
            raise click.Abort
        out_dir = generate_plat_cmake(target)
        shutil.copy(
            os.path.join(PAL_PLATFORM_ROOT, 'mbedCloudClientCmake.txt'),
            os.path.join(out_dir, 'CMakeLists.txt')
        )
        click.echo(
            click.style(
                'Generation for %s is successful, please run cmake & make from %s' % (config.target_name, out_dir),
                fg='green'
            )
        )


@cli.command(
    context_settings=CONTEXT_SETTINGS,
    short_help='Clean platform-dependent files (run "%s clean -h" for help)' % PROG_NAME
)
@click.option(
    '--target',
    'target_name',
    help='The target to clean',
    required=True,
    type=DynamicChoice(get_available_targets)
)
@click.option(
    '-k', '--keep-sources', is_flag=True,
    help='Keep the deployed platform-dependent files (clean only generated files)'
)
@pass_config
def clean(config, target_name, keep_sources):
    """Clean target-dependent files"""
    config.target_name = target_name

    target = Target(config.target_name, config.targets[target_name], config.stream_kwargs)
    out_dir_name = '__' + target.name
    parent_dir = os.path.normpath(os.path.join(PAL_PLATFORM_ROOT, os.pardir))
    out_dir = os.path.join(parent_dir, out_dir_name)

    if os.path.isdir(out_dir):
        logger.info('Deleting %s', out_dir)
        shutil.rmtree(out_dir, onerror=del_rw)

    if not keep_sources:
        target.delete_elements()


def runCmakeAndMake(folder, debug, toolchain, outdir, envPair, external, name, numOfBuildThreads):
    logger.info('running cmake')
    #"""cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/ARMGCC/ARMGCC.cmake -DEXTARNAL_DEFINE_FILE=../mbed-client-pal/Examples/PlatformBSP/mbedTLS/mbedTLS_cmake_config.txt"""
    command = "cmake"
    make_params = "-j1"
    argument1 = "-G"
    argument2 = "Unix Makefiles"
    if debug == 1:
        argument3 = "-DCMAKE_BUILD_TYPE=Debug"
    else:
        argument3 = "-DCMAKE_BUILD_TYPE=Release"
    argument4 = "-DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/"+toolchain+"/"+toolchain+".cmake"
    if external != None:
        argument5 = "-DEXTARNAL_DEFINE_FILE="+external
    else:
        argument5 = "-DEXTARNAL_DEFINE_FILE=../mbed-client-pal/Configs/pal_ext_configs.cmake"
    if envPair != None:
        logger.info('setting environment: %s = %s', envPair[0], envPair[1])
        os.environ[envPair[0]] = envPair[1]
    if numOfBuildThreads != None:
        make_params = "-j" + str(numOfBuildThreads)

    p = subprocess.Popen([command, argument1, argument2, argument3, argument4, argument5], cwd=folder)
    returnCode = p.wait()
    if returnCode == 0:
        if name != None:
            p = subprocess.Popen(["make", make_params, name], cwd=folder)
        else:
            p = subprocess.Popen(["make", make_params], cwd=folder)
        returnCode = p.wait()
        if returnCode == 0:
            if debug == 1:
                copyFrom = os.path.join(folder, "Debug")
            else:
                copyFrom = os.path.join(folder, "Release")
            if not os.path.exists(outdir):
                os.makedirs(outdir)
            shutil.move(copyFrom, outdir)

        else:
            logger.error("** build failed **")
    else:
        logger.error("** CMAKE failed **")


def getPathForToolChainInPath(toolchain):
    output = None
    realname = toolchain
    if platform.system() == 'Windows': #widnows type OS
        realname = toolchain + ".exe"
        try:
            found = check_output_and_raise(['where', realname]).strip()
        except Exception as e:
            logger.error(e)
            logger.error("** Toolchain %s not found in path  - make sure toolchain executable [%s] is in the path **", toolchain, realname)
            return None
        separator = "\\"
        double_separator = "\\\\"
    else: # assume linux type OS
        try:
            found = check_output_and_raise(['which', realname]).strip()
        except Exception as e:
            logger.error(e)
            logger.error("** Toolchain %s not found in path  - make sure toolchain executable [%s] is in the path **", toolchain, realname)
            return None
        separator = "/"
        double_separator = "//"
    if found != None:
        parent_dir = found #os.path.normpath(os.path.join(PAL_PLATFORM_ROOT, os.pardir))
        while parent_dir.endswith(separator+realname) or parent_dir.endswith(separator+realname) or parent_dir.endswith(separator+"bin") or parent_dir.endswith(separator+"bin"+separator) or parent_dir.endswith(double_separator+"bin"+separator) or parent_dir.endswith(separator+"bin"+double_separator) or parent_dir.endswith(double_separator+"bin"+double_separator):
            parent_dir = os.path.normpath(os.path.join(parent_dir, os.pardir))
        output = parent_dir
    return output


def checkToolchainEnv(toolchain):
    logger.info('Checking Environment for Toolchain - %s', toolchain)
    #toolchain_env stucture: key == toolchain name , value is a tupple  with two elements:
    #1. a tuple of relevant environment variables for the toolchain - Note : the first value is the one we want (we will export it if any of the values are found)
    #2. a string with the expected name of compiler binary for path seach
    toolchainEnv = {"ARMCC":(("ARMCC_DIR", "ARM_PATH", "MBED_ARM_PATH"), "armcc"),
                    "ARMGCC":(("ARMGCC_DIR", "GCC_ARM_PATH", "MBED_GCC_ARM_PATH"), "arm-none-eabi-gcc"),
                    "GCC": (("GCC_DIR",), "gcc"),
                    "GCC-OPENWRT": (("TOOLCHAIN_DIR",), "arm-openwrt-linux-gcc")}

    toolchainInfo = toolchainEnv.get(toolchain, None)
    if None == toolchainInfo:
        logger.warning('toolchain environment not found for toolchain selected [%s] - please make sure toolchain is present and the correct environment variable is set', toolchain)
        return None

    for envVariable in toolchainInfo[0]:
        path = os.getenv(envVariable, None)
        if path != None:
            logger.debug("env variable %s found", envVariable)
            return (toolchainInfo[0][0], path)
    path = getPathForToolChainInPath(toolchainInfo[1])
    if path != None:
        return (toolchainInfo[0][0], path)

    logger.warning('toolchain environment not found for toolchain selected [%s] - please make sure toolchain is present and correct environment variable is set [%s]', toolchain, toolchainInfo[0][0])
    return None


@cli.command(
    context_settings=CONTEXT_SETTINGS,
    short_help='[DEPRECATED] fullBuild deploy and build the project (run "%s  fullBuild -h" for help)' % PROG_NAME)
@click.option(
    '--target',
    'target_name',
    help='The target to deploy and build',
    required=True,
    type=DynamicChoice(get_available_targets)
)
@click.option(
    '--toolchain',
    'toolchain',
    help='The toolchain to use for the build',
    required=True,
    type=DynamicChoice(get_available_toolchains)
)
@click.option(
    '--external',
    'external',
    help='The path of the eternal define CMAKE file to include',
    required=False,
    type=click.Path()
)
@click.option(
    '--name',
    'name',
    help='name of the build target passed to the make command',
    required=False,
    type=click.Path()
)
@click.option(
    '-k', '--keep-sources', is_flag=True,
    help='Keep the deployed platform-dependent files (clean only generated files)'
)
@click.option(
    '-j',
    'numOfBuildThreads',
    help='-j parallel make parameter (Example: -j4)',
    required=False,
    type=int
)
@pass_config
def fullbuild(config, target_name, toolchain, external, name, keep_sources, numOfBuildThreads):
    """deploy and build target files"""
    config.target_name = target_name
    logger.info('fullBuild option has been DEPRECATED and will be removed in future release.')
    logger.info('fullBuild running for target = %s with toolchain = %s', target_name, toolchain)

    ctx = click.get_current_context()
    ctx.invoke(deploy, target_name=target_name, skip_update=None, instructions=None)
    ctx.invoke(generate, target_name=target_name)

    envPair = checkToolchainEnv(toolchain)
    if (None == envPair):
        logger.error("** Toolchain not found - exiting **")
        return
    target = Target(config.target_name, config.targets[target_name], config.stream_kwargs)
    out_dir_name = '__' + target.name
    parent_dir = os.path.normpath(os.path.join(PAL_PLATFORM_ROOT, os.pardir))
    out_dir = os.path.join(parent_dir, out_dir_name)
    isDebug = 1 # build debug version
    output = os.path.join(parent_dir, "out")
    if os.path.exists(output):
        shutil.rmtree(output)

    runCmakeAndMake(out_dir, isDebug, toolchain, output, envPair, external, name, numOfBuildThreads)  # CMAKE + build debug version

    isDebug = 0 # generate and build release version
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    ctx.invoke(generate, target_name=target_name)

    runCmakeAndMake(out_dir, isDebug, toolchain, output, envPair, external, name, numOfBuildThreads)  # CMAKE + build release version

    logger.info('fullBuild option has been DEPRECATED and will be removed in future release.')
    logger.info('\nCompleted fullBuild running for target = %s\nWith toolchain = %s.\nOutput directory: %s\n', target_name, toolchain, output)


if __name__ == '__main__':
    cli(sys.argv[1:])
