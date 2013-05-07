/*
 * Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
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

#ifndef f_engine_utils_h
#define f_engine_utils_h

#include "base.h"
#include "pipeline.h"
#include "errorhandlers.h"
#include "ftlib.h"
#include <math.h>




/* -----------------------------------------------------------------------*/
/*                                generic                                 */
/* -----------------------------------------------------------------------*/

int
get_fd(char* filename);





/* -----------------------------------------------------------------------*/
/*                         query parsing utilities                        */
/* -----------------------------------------------------------------------*/

uint64_t
get_enum(const char * const name);

/* -----------------------------------------------------------------------*/









/* -----------------------------------------------------------------------*/
/*                         grouper utilities                              */
/* -----------------------------------------------------------------------*/



/* quick sort comparator */
#if defined (__APPLE__) || defined (__FreeBSD__)
  int comp_uint8_t(void *thunk, const void *e1, const void *e2);
  int comp_uint16_t(void *thunk, const void *e1, const void *e2);
  int comp_uint32_t(void *thunk, const void *e1, const void *e2);
  int comp_uint64_t(void *thunk, const void *e1, const void *e2);
#elif defined(__linux)
  int comp_uint8_t(const void *e1, const void *e2, void* thunk);
  int comp_uint16_t(const void *e1, const void *e2, void* thunk);
  int comp_uint32_t(const void *e1, const void *e2, void* thunk);
  int comp_uint64_t(const void *e1, const void *e2, void* thunk);
#endif

/* -----------------------------------------------------------------------*/









/* -----------------------------------------------------------------------*/
/*                         merger utilities                               */
/* -----------------------------------------------------------------------*/

struct permut_iter {
  size_t                          num_branches;
  size_t*                         num_filtered_groups;
  size_t*                         filtered_group_tuple;
};

struct permut_iter*
iter_init(
          size_t num_branches,
          struct branch** const branchset
          );
bool
iter_next(const struct permut_iter *iter);

void
iter_destroy(struct permut_iter *iter);

/* -----------------------------------------------------------------------*/

#endif
