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

#include "bsdiff.h"
#include "varint.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlong-long"
#include "lz4.h"
#pragma GCC diagnostic pop

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX_FRAME_SIZE_DEFAULT 512
//#define TRACING
#ifdef TRACING
#define log(...) printf(__VA_ARGS__)
#else
#define log(...)
#endif

static void split(int64_t *I, int64_t *V, int64_t start, int64_t len, int64_t h)
{
    int64_t i, j, k, x, tmp, jj, kk;

    if (len < 16) {
        for (k = start; k < start + len; k += j) {
            j = 1;
            x = V[I[k] + h];
            for (i = 1; k + i < start + len; i++) {
                if (V[I[k + i] + h] < x) {
                    x = V[I[k + i] + h];
                    j = 0;
                };
                if (V[I[k + i] + h] == x) {
                    tmp = I[k + j];
                    I[k + j] = I[k + i];
                    I[k + i] = tmp;
                    j++;
                };
            };
            for (i = 0; i < j; i++){
                V[I[k + i]] = k + j - 1;
            }
            if (j == 1){
                I[k] = -1;
            }
        };
        return;
    };

    x = V[I[start + len / 2] + h];
    jj = 0;
    kk = 0;
    for (i = start; i < start + len; i++) {
        if (V[I[i] + h] < x){
            jj++;
        }
        if (V[I[i] + h] == x){
            kk++;
        }
    };
    jj += start;
    kk += jj;

    i = start;
    j = 0;
    k = 0;
    while (i < jj) {
        if (V[I[i] + h] < x) {
            i++;
        } else if (V[I[i] + h] == x) {
            tmp = I[i];
            I[i] = I[jj + j];
            I[jj + j] = tmp;
            j++;
        } else {
            tmp = I[i];
            I[i] = I[kk + k];
            I[kk + k] = tmp;
            k++;
        };
    };

    while (jj + j < kk) {
        if (V[I[jj + j] + h] == x) {
            j++;
        } else {
            tmp = I[jj + j];
            I[jj + j] = I[kk + k];
            I[kk + k] = tmp;
            k++;
        };
    };

    if (jj > start){
        split(I, V, start, jj - start, h);
    }

    for (i = 0; i < kk - jj; i++){
        V[I[jj + i]] = kk - 1;
    }
    if (jj == kk - 1){
        I[jj] = -1;
    }

    if (start + len > kk){
        split(I, V, kk, start + len - kk, h);
    }
}

static void qsufsort(int64_t *I, int64_t *V, const uint8_t *old, int64_t oldsize)
{
    int64_t buckets[256];
    int64_t i, h, len;

    for (i = 0; i < 256; i++)
        buckets[i] = 0;
    for (i = 0; i < oldsize; i++)
        buckets[old[i]]++;
    for (i = 1; i < 256; i++)
        buckets[i] += buckets[i - 1];
    for (i = 255; i > 0; i--)
        buckets[i] = buckets[i - 1];
    buckets[0] = 0;

    for (i = 0; i < oldsize; i++)
        I[++buckets[old[i]]] = i;
    I[0] = oldsize;
    for (i = 0; i < oldsize; i++)
        V[i] = buckets[old[i]];
    V[oldsize] = 0;
    for (i = 1; i < 256; i++)
        if (buckets[i] == buckets[i - 1] + 1)
            I[buckets[i]] = -1;
    I[0] = -1;

    for (h = 1; I[0] != -(oldsize + 1); h += h) {
        len = 0;
        for (i = 0; i < oldsize + 1;) {
            if (I[i] < 0) {
                len -= I[i];
                i -= I[i];
            } else {
                if (len)
                    I[i - len] = -len;
                len = V[I[i]] + 1 - i;
                split(I, V, i, len, h);
                i += len;
                len = 0;
            };
        };
        if (len)
            I[i - len] = -len;
    };

    for (i = 0; i < oldsize + 1; i++)
        I[V[i]] = i;
}

static int64_t matchlen(const uint8_t *old, int64_t oldsize, const uint8_t *new, int64_t newsize)
{
    int64_t i;

    for (i = 0; (i < oldsize) && (i < newsize); i++)
        if (old[i] != new[i])
            break;

    return i;
}

static int64_t search(const int64_t *I, const uint8_t *old, int64_t oldsize, const uint8_t *new, int64_t newsize,
        int64_t st, int64_t en, int64_t *pos)
{
    int64_t x, y;

    if (en - st < 2) {
        x = matchlen(old + I[st], oldsize - I[st], new, newsize);
        y = matchlen(old + I[en], oldsize - I[en], new, newsize);

        if (x > y) {
            *pos = I[st];
            return x;
        } else {
            *pos = I[en];
            return y;
        }
    };

    x = st + (en - st) / 2;
    if (memcmp(old + I[x], new, MIN(oldsize - I[x], newsize)) < 0) {
        return search(I, old, oldsize, new, newsize, x, en, pos);
    } else {
        return search(I, old, oldsize, new, newsize, st, x, pos);
    };
}

static void offtout(int64_t x, uint8_t *buf)
{
    int64_t y;

    if (x < 0)
        y = -x;
    else
        y = x;

    buf[0] = y % 256;
    y -= buf[0];
    y = y / 256;
    buf[1] = y % 256;
    y -= buf[1];
    y = y / 256;
    buf[2] = y % 256;
    y -= buf[2];
    y = y / 256;
    buf[3] = y % 256;
    y -= buf[3];
    y = y / 256;
    buf[4] = y % 256;
    y -= buf[4];
    y = y / 256;
    buf[5] = y % 256;
    y -= buf[5];
    y = y / 256;
    buf[6] = y % 256;
    y -= buf[6];
    y = y / 256;
    buf[7] = y % 256;

    if (x < 0)
        buf[7] |= 0x80;
}

/**
 * Write given buffer to stream in frames, with MAX_FRAME_SIZE as maximum deCompressBuffer and undeCompressBuffer size.
 * Store the biggest deCompressBuffer frame size to max_deCompressBuffer_size, if it's bigger than current value.
 * @return 0 on success, negative on error.
 */
static int writedeCompressBuffer(struct bsdiff_stream* stream, const void* buffer, int64_t length,
        int64_t* max_deCompressBuffer_size, int64_t max_frame_size)
{
    int src_ptr;
    char* temp;
    int deCompressBuffer_size;
    int written = 0;
    uint8_t buf[8];

    if (length == 0)
        return 0;

    temp = stream->malloc(max_frame_size);
    src_ptr = MIN(length, max_frame_size);

    do {
        deCompressBuffer_size = LZ4_compress_destSize(buffer, temp, &src_ptr, max_frame_size);

        if (deCompressBuffer_size == 0) {
            stream->free(temp);
            return -1;
        }

        int encodedSize = encode_unsigned_varint(deCompressBuffer_size, buf, sizeof(buf));

        log("writedeCompressBuffer %llu compressedSize %u (var intS %u)\n", length, deCompressBuffer_size, encodedSize);
        if (encodedSize <= 0) {
            return encodedSize;  // error in encoding
        }
        stream->write(stream, buf, encodedSize);

        stream->write(stream, temp, deCompressBuffer_size);

        buffer = (char*) buffer + src_ptr;
        written += src_ptr;

        src_ptr = MIN(length - written, max_frame_size);

        if (deCompressBuffer_size > *max_deCompressBuffer_size){
            *max_deCompressBuffer_size = deCompressBuffer_size;
        }

    } while (written != length);

    stream->free(temp);
    return 0;
}

struct bsdiff_request {
    const uint8_t* old;
    int64_t oldsize;
    const uint8_t* new;
    int64_t newsize;
    struct bsdiff_stream* stream;
    int64_t *I;
    uint8_t *buffer;
};

static int bsdiff_internal(const struct bsdiff_request req, int64_t* max_deCompressBuffer_size,
        const int64_t max_frame_size)
{
    int64_t *I, *V;
    int64_t scan, pos, len;
    int64_t lastscan, lastpos, lastoffset;
    int64_t oldscore, scsc;
    int64_t s, Sf, lenf, Sb, lenb;
    int64_t overlap, Ss, lens;
    int64_t i;
    uint8_t *buffer;
    uint8_t buf[8 * 3];

    if ((V = req.stream->malloc((req.oldsize + 1) * sizeof(int64_t))) == NULL){
        return -1;
    }
    I = req.I;

    qsufsort(I, V, req.old, req.oldsize);
    req.stream->free(V);

    buffer = req.buffer;

    /* Compute the differences, writing ctrl as we go */
    scan = 0;
    len = 0;
    pos = 0;
    lastscan = 0;
    lastpos = 0;
    lastoffset = 0;
    while (scan < req.newsize) {
        oldscore = 0;

        for (scsc = scan += len; scan < req.newsize; scan++) {
            len = search(I, req.old, req.oldsize, req.new + scan, req.newsize - scan, 0, req.oldsize, &pos);

            for (; scsc < scan + len; scsc++)
                if ((scsc + lastoffset < req.oldsize) && (req.old[scsc + lastoffset] == req.new[scsc]))
                    oldscore++;

            if (((len == oldscore) && (len != 0)) || (len > oldscore + 8))
                break;

            if ((scan + lastoffset < req.oldsize) && (req.old[scan + lastoffset] == req.new[scan]))
                oldscore--;
        };

        if ((len != oldscore) || (scan == req.newsize)) {
            s = 0;
            Sf = 0;
            lenf = 0;
            for (i = 0; (lastscan + i < scan) && (lastpos + i < req.oldsize);) {
                if (req.old[lastpos + i] == req.new[lastscan + i])
                    s++;
                i++;
                if (s * 2 - i > Sf * 2 - lenf) {
                    Sf = s;
                    lenf = i;
                };
            };

            lenb = 0;
            if (scan < req.newsize) {
                s = 0;
                Sb = 0;
                for (i = 1; (scan >= lastscan + i) && (pos >= i); i++) {
                    if (req.old[pos - i] == req.new[scan - i])
                        s++;
                    if (s * 2 - i > Sb * 2 - lenb) {
                        Sb = s;
                        lenb = i;
                    };
                };
            };

            if (lastscan + lenf > scan - lenb) {
                overlap = (lastscan + lenf) - (scan - lenb);
                s = 0;
                Ss = 0;
                lens = 0;
                for (i = 0; i < overlap; i++) {
                    if (req.new[lastscan + lenf - overlap + i] == req.old[lastpos + lenf - overlap + i])
                        s++;
                    if (req.new[scan - lenb + i] == req.old[pos - lenb + i])
                        s--;
                    if (s > Ss) {
                        Ss = s;
                        lens = i + 1;
                    };
                };

                lenf += lens - overlap;
                lenb -= lens;
            };

            int64_t diff_str_len = lenf;
            int64_t extra_str_len_y = (scan - lenb) - (lastscan + lenf);
            int64_t old_file_ctrl_off_set_jump = (pos - lenb) - (lastpos + lenf);

            int posInBuf = 0;
            int encodedSize = encode_unsigned_varint(diff_str_len, &buf[posInBuf], 8);

            assert(encodedSize > 0 && encodedSize <= 8);
            posInBuf += encodedSize;

            encodedSize = encode_unsigned_varint(extra_str_len_y, &buf[posInBuf], 8);
            assert(encodedSize > 0 && encodedSize <= 8);
            posInBuf += encodedSize;

            encodedSize = encode_signed_varint(old_file_ctrl_off_set_jump, &buf[posInBuf], 8);
            assert(encodedSize > 0 && encodedSize <= 8);
            posInBuf += encodedSize;

            log( "DIFF_STR_LEN_X: %lld EXTRA_STR_LEN_Y: %lld OLD_FILE_CTRL_OFF_SET_JUMP %lld (encoded to %i)\n",
                    lenf, (scan - lenb) - (lastscan + lenf), (pos - lenb) - (lastpos + lenf), posInBuf);

            /* Write control data */
            req.stream->write(req.stream, buf, posInBuf);

            /* Write diff data */
            for (i = 0; i < lenf; i++){
                buffer[i] = req.new[lastscan + i] - req.old[lastpos + i];
            }
            if (writedeCompressBuffer(req.stream, buffer, lenf, max_deCompressBuffer_size, max_frame_size)) {
                return -1;
            }

            /* Write extra data */
            for (i = 0; i < (scan - lenb) - (lastscan + lenf); i++){
                buffer[i] = req.new[lastscan + lenf + i];
            }
            if (writedeCompressBuffer(req.stream, buffer, (scan - lenb) - (lastscan + lenf), max_deCompressBuffer_size,
                    max_frame_size)) {
                return -1;
            }

            lastscan = scan - lenb;
            lastpos = pos - lenb;
            lastoffset = pos - scan;
        };
    };

    return 0;
}

int bsdiff(const uint8_t* old, int64_t oldsize, const uint8_t* new, int64_t newsize, struct bsdiff_stream* stream,
        int64_t* max_deCompressBuffer_size, const int64_t max_frame_size)
{
    int result;
    struct bsdiff_request req;

    if ((req.I = stream->malloc((oldsize + 1) * sizeof(int64_t))) == NULL) {
        return -1;
    }

    if ((req.buffer = stream->malloc(newsize + 1)) == NULL) {
        stream->free(req.I);
        return -1;
    }

    req.old = old;
    req.oldsize = oldsize;
    req.new = new;
    req.newsize = newsize;
    req.stream = stream;

    result = bsdiff_internal(req, max_deCompressBuffer_size, max_frame_size);

    stream->free(req.buffer);
    stream->free(req.I);

    return result;
}

#if defined(BSDIFF_EXECUTABLE)

#include <sys/types.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int file_write(struct bsdiff_stream* stream, const void* buffer,
        uint64_t size) {
    return fwrite(buffer, 1, size, (FILE*) stream->opaque) == size ? 0 : -1;
}

int main(int argc, char *argv[]) {
    int fd;
    uint8_t *old, *new;
    off_t oldsize, newsize;
    uint8_t buf[24];
    FILE * pf;
    struct bsdiff_stream stream;
    int64_t max_deCompressBuffer_size = 0;
    int64_t patch_file_size = 0;
    int64_t max_frame_size = MAX_FRAME_SIZE_DEFAULT;

    stream.malloc = malloc;
    stream.free = free;
    stream.write = file_write;

    if (argc < 4 || argc > 5) {
        errx(1, "usage: %s oldfile newfile patchfile <max_frame_size>\n",
                argv[0]);
    }

    if (argc == 5) {
        max_frame_size = strtol(argv[4], 0, 10);
    } else {
        printf("Using default max frame size of %d.\n", MAX_FRAME_SIZE_DEFAULT);
    }

    if (max_frame_size < 64) {
        err(1, "Please define max frame size as integer >= 64");
    }

    /* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
     that we never try to malloc(0) and get a NULL pointer */
    if (((fd = open(argv[1], O_RDONLY, 0)) < 0)
            || ((oldsize = lseek(fd, 0, SEEK_END)) == -1)
            || ((old = malloc(oldsize + 1)) == NULL)
            || (lseek(fd, 0, SEEK_SET) != 0)
            || (read(fd, old, oldsize) != oldsize) || (close(fd) == -1)) {
        err(1, "%s", argv[1]);
    }

    /* Allocate newsize+1 bytes instead of newsize bytes to ensure
     that we never try to malloc(0) and get a NULL pointer */
    if (((fd = open(argv[2], O_RDONLY, 0)) < 0)
            || ((newsize = lseek(fd, 0, SEEK_END)) == -1)
            || ((new = malloc(newsize + 1)) == NULL)
            || (lseek(fd, 0, SEEK_SET) != 0)
            || (read(fd, new, newsize) != newsize) || (close(fd) == -1)) {
        err(1, "%s", argv[2]);
    }

    /* Create the patch file */
    if ((pf = fopen(argv[3], "w")) == NULL) {
        err(1, "%s", argv[3]);
    }

    /* Write header (signature+newsize+max undeCompressBuffer+maxdeCompressBuffer)*/
    offtout(newsize, buf);
    offtout(max_frame_size, buf + 8);
    offtout(max_deCompressBuffer_size, buf + 16);
    if (fwrite("PELION/BSDIFF001", 16, 1, pf) != 1
            || fwrite(buf, sizeof(buf), 1, pf) != 1)
    err(1, "Failed to write header");

    stream.opaque = pf;
    if (bsdiff(old, oldsize, new, newsize, &stream, &max_deCompressBuffer_size,
                    max_frame_size)) {
        err(1, "bsdiff");
    }

    /* Go back to header and fill the maxdeCompressBuffer properly */
    offtout(max_deCompressBuffer_size, buf);
    fseek(pf, 32, SEEK_SET);
    fwrite(buf, 1, 8, pf);

    fseek(pf, 0, SEEK_END);
    patch_file_size = ftell(pf);

    if (fclose(pf)) {
        err(1, "fclose");
    }

    printf(
            "Wrote diff file %s, size %ld. Max undeCompressBuffer frame size was %ld, max deCompressBuffer frame size was %ld.\n",
            argv[3], patch_file_size, max_frame_size,
            max_deCompressBuffer_size);

    /* Free the memory we used */
    free(old);
    free(new);

    return 0;
}

#endif
