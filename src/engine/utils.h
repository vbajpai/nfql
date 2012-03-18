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

#ifndef flowy_engine_utils_h
#define flowy_engine_utils_h

#include "base.h"
#include "error_functions.h"






/* -----------------------------------------------------------------------*/  
/*                         grouper utilities                              */
/* -----------------------------------------------------------------------*/

struct bsearch_handle {
  char **ordered_records;
  char ***uniq_records;
  size_t num_uniq_records;
};

void *
bsearch_r(const void *key, 
          const void *base, 
          size_t nmemb, 
          size_t size,
          void *thunk,
          int (*compar) (void *thunk, const void *, const void *));

/* -----------------------------------------------------------------------*/  









/* -----------------------------------------------------------------------*/  
/*                         merger utilities                               */
/* -----------------------------------------------------------------------*/

struct permut_iter {
  size_t num_branches;
  size_t *num_filtered_groups;
  size_t *filtered_group_tuple;
};

struct permut_iter *
iter_init(size_t *lengths, size_t arr_len);

bool 
iter_next(struct permut_iter *iter);

void 
iter_destroy(struct permut_iter *iter);

/* -----------------------------------------------------------------------*/  

#endif
