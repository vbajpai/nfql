/*
 * Copyright 2011 Johannes 'josch' Schauer <j.schauer@email.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ftreader.h"
#include "flowy.h"

/*
 * pointer hell...
 *
 * used for qsort_r and bsearch_r
 * using http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
 * casting to long because subtraction of unsigned integer
 */

/*
 * used for qsort_r
 */
#define comp(size) \
int comp_##size(const void *e1, const void *e2, void *thunk) \
{ \
    size x, y; \
    x = *(size *)(*(char **)e1+*(unsigned short *)thunk); \
    y = *(size *)(*(char **)e2+*(unsigned short *)thunk); \
    return (x > y) - (y > x); \
}

comp(uint8_t);
comp(uint16_t);
comp(uint32_t);
comp(uint64_t);

/*
 * used for bsearch_r
 */
#define comp_p(size) \
int comp_##size##_p(const void *e1, const void *e2, void *thunk) \
{ \
    size x, y; \
    x = *(size *)((char *)e1+*(unsigned short *)thunk); \
    y = *(size *)(**(char ***)e2+*(unsigned short *)thunk); \
    return (x > y) - (y > x); \
}

comp_p(uint8_t);
comp_p(uint16_t);
comp_p(uint32_t);
comp_p(uint64_t);

/*
 * returns a struct bsearch_handle which contains
 */
#define tree_create(size) \
struct bsearch_handle *tree_create_##size(char **records, size_t num_records, unsigned short field_offset) \
{ \
    struct bsearch_handle *handle; \
    int i; \
\
    handle = (struct bsearch_handle *)malloc(sizeof(struct bsearch_handle)); \
    if (handle == NULL) { \
        perror("malloc"); \
        exit(EXIT_FAILURE); \
    } \
\
    memset(handle, 0, sizeof(struct bsearch_handle)); \
\
    handle->ordered_records = (char **)malloc(num_records*sizeof(char *)); \
    if (handle->ordered_records == NULL) { \
        perror("malloc"); \
        exit(EXIT_FAILURE); \
    } \
\
    memcpy(handle->ordered_records, records, num_records*sizeof(char *)); \
    qsort_r(handle->ordered_records, num_records, sizeof(char *), comp_##size, (void *)&field_offset); \
\
    handle->uniq_records = (char ***)malloc(num_records*sizeof(char **)); \
    handle->uniq_records[0] = &handle->ordered_records[0]; \
    handle->num_uniq_records = 1; \
    for (i = 1; i < num_records; i++) { \
        if (*(size *)(handle->ordered_records[i]+field_offset) \
         != *(size *)(*handle->uniq_records[handle->num_uniq_records-1]+field_offset)) { \
            handle->uniq_records[handle->num_uniq_records++] = &handle->ordered_records[i]; \
        } \
    } \
    handle->uniq_records = realloc(handle->uniq_records, \
                                   handle->num_uniq_records*sizeof(char **)); \
\
    return handle; \
}

tree_create(uint8_t);
tree_create(uint16_t);
tree_create(uint32_t);
tree_create(uint64_t);

#define tree_find(size) \
char **tree_find_##size(struct bsearch_handle *handle, \
                        char *record, \
                        unsigned short field_offset) \
{ \
    return *(char ***)bsearch_r(record, \
            (void *)handle->uniq_records, \
            handle->num_uniq_records, \
            sizeof(char **), \
            comp_##size##_p, \
            (void *)&field_offset \
            ); \
}

tree_find(uint8_t);
tree_find(uint16_t);
tree_find(uint32_t);
tree_find(uint64_t);

void tree_destroy(struct bsearch_handle *handle)
{
    free(handle->ordered_records);
    free(handle->uniq_records);
    free(handle);
}
