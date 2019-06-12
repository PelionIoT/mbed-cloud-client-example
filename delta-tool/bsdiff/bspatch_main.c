/*-
 * Copyright 2003-2005 Colin Percival
 * Copyright 2012 Matthew Endsley
 * Copyright (c) 2018-2019 ARM Limited
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "bspatch.h"

#define MAX_NEXT_SIZE 512

int64_t batch_read_size;

static int virtualDataAmount = 0;
static int nextEventToPostToBsPatch = 0;
static int readCounter = 0;

static int seek_old(const struct bspatch_stream* stream, int64_t seek_diff) {

    FILE* f = ((FILE**) (ARM_BS_GetOpaque(stream)))[1];
    assert("failure in seek" && fseek(f, seek_diff, SEEK_CUR)==0);

    nextEventToPostToBsPatch = EBSAPI_SEEK_OLD_DONE;
    return EBSAPI_OPERATION_OLD_FILE_SEEK_WILL_COMPLETE_LATER;
}

static int read_old(const struct bspatch_stream* stream, void* buffer,
                    uint64_t length) {
    FILE* f = ((FILE**) ( ARM_BS_GetOpaque(stream) ))[1];
    unsigned int read = (unsigned int)fread(buffer, 1, length, f);
    if ( read != length)
        printf("Read old err: %u (%lu) - %ld\n", read, length,
               ftell(f));

    assert(read == length);
    if( readCounter % 2 == 0) {
        nextEventToPostToBsPatch = EBSAPI_READ_OLD_DONE;
        return EBSAPI_OPERATION_OLD_FILE_READ_WILL_COMPLETE_LATER;
    } else {
        return EBSAPI_OPERATION_DONE_IMMEDIATELY;
    }
}

int PATCH_TRACING = 0;
static int read_patch(const struct bspatch_stream* stream, void* buffer,
                      uint64_t length) {
    readCounter ++;
    virtualDataAmount -= length;
    int read;
    batch_read_size -= length;
    FILE* f = ((FILE**) ARM_BS_GetOpaque(stream))[0];
    read = fread(buffer, 1, length, f);

    if (read < 0 || (unsigned int) read != length)
        printf("Read patch err: %d (%d)\n", read, 0);

    assert(nextEventToPostToBsPatch == 0);

    if(PATCH_TRACING) {
        printf(" (%u)B\n",(uint32_t)length);
        PATCH_TRACING = 0;
    }
    // lets fake later completion here every other time
    if( readCounter % 2 == 0) {
        nextEventToPostToBsPatch = EBSAPI_READ_PATCH_DONE;
        return EBSAPI_OPERATION_PATCH_READ_WILL_COMPLETE_LATER;
    } else {
        return EBSAPI_OPERATION_DONE_IMMEDIATELY;
    }
}

static int write_new(const struct bspatch_stream* stream, void* buffer,
                     uint64_t length) {

    FILE* f = ((FILE**) ARM_BS_GetOpaque(stream))[2];
    assert(fwrite(buffer, 1, length, f) == length);

    if( readCounter % 2 == 0) {
        nextEventToPostToBsPatch = EBSAPI_WRITE_NEW_DONE;
        return  EBSAPI_OPERATION_NEW_FILE_WRITE_WILL_COMPLETE_LATER;
    } else {
        return EBSAPI_OPERATION_DONE_IMMEDIATELY;
    }

}

// lets make this global to have size better visible in compile time
struct bspatch_stream bs_patch;

#include "bspatch_private.h"
int main(int argc, char * argv[]) {
    FILE* f[3];

    int status;

    if (argc != 4)
        errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);

    /* Open patch file */
    if ((f[0] = fopen(argv[3], "rb")) == NULL)
        err(1, "fopen(%s)", argv[3]);

    /* Open old file */
    if ((f[1] = fopen(argv[1], "rb")) == NULL)
        err(1, "fopen(%s)", argv[1]);

    /* Open new file */
    if ((f[2] = fopen(argv[2], "wbx")) == NULL)
        err(1, "fopen(%s)", argv[2]);

    ARM_BS_Init(&bs_patch, f,
                read_patch,
                read_old,
                seek_old,
                write_new );

    printf("BSPatch Size: %lu B\n", sizeof(bs_patch));

    /* Run bspatching */
    status = ARM_BS_ProcessPatchEvent(&bs_patch, EBSAPI_START_PATCH_PROCESSING);
    if(status != EBSAPI_PATCH_DONE) {
        do {
            assert(nextEventToPostToBsPatch);
            bs_patch_api_event_t eventToPost = nextEventToPostToBsPatch;
            nextEventToPostToBsPatch = 0; // zero the global to make asserts on implementation to work. assumes to be zero when new request arrives
            status = ARM_BS_ProcessPatchEvent(&bs_patch, eventToPost);
        } while (status > 0 && status != EBSAPI_PATCH_DONE);
    }

    status = ARM_BS_Free(&bs_patch);

    if (status != 0)
        errx(1, "bspatch fail: %d", status);

    /* close file handles */
    fclose(f[2]);
    fclose(f[1]);
    fclose(f[0]);

    return 0;
}

