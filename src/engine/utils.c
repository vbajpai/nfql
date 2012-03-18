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

#include "utils.h"

/*
 * adapted from ./stdlib/bsearch.c from the GNU C Library
 * pass additional argument thunk to compar to allow it access additional data
 * without global variables - in our case, we need to pass the data offset
 */
void *
bsearch_r(const void *key, 
          const void *base, 
          size_t nmemb, 
          size_t size,
          void *thunk,
          int (*compar) (void *thunk, const void *, const void *)) {
  size_t l, u, idx;
  const void *p;
  int comparison;
  
  l = 0;
  u = nmemb;
  while (l < u) {
    idx = (l + u) / 2;
    p = (void *) (((const char *) base) + (idx * size));
    comparison = (*compar) (thunk, key, p);
    if (comparison < 0) {
      u = idx;
    } else if (comparison > 0) {
      l = idx + 1;
    } else {
      return (void *) p;
    }
  }
  return NULL;
}


/*
 * those functions just go through all possible permutations of sequential
 * numbers from N lists. Each of those lists is parameterized by an offset
 * (the beginning of the sequence) and a length.
 *
 * The lists are not stored but only the current permutation and the list
 * properties are carried along.
 */

/*
 * initialize a new permutation vector, given the initial offsets
 */
struct permut_iter *
iter_init(size_t *lengths, size_t arr_len) 
{ 
  struct permut_iter *iter;
  int i;
  
  iter = (struct permut_iter *)malloc(sizeof(struct permut_iter));
  if (iter == NULL)
    errExit("malloc");
  
  iter->len = arr_len;  
  iter->array = (size_t *)malloc(sizeof(size_t)*arr_len);  
  if (iter->array == NULL)
    errExit("malloc");  
  iter->lengths = (size_t *)malloc(sizeof(size_t)*arr_len);  
  if (iter->lengths == NULL)
    errExit("malloc");
  
  for (i = 0; i < arr_len; i++) {
    iter->array[i] = 1;
    iter->lengths[i] = lengths[i];
  }
  
  return iter;
}

/*
 * given the iterator, modify its 'array' permutation vector to represent the
 * next permutation and return 1. If the last permutation is reached, return
 * 0.
 */
int iter_next(struct permut_iter *iter)
{
  int i;
  
  // count up by one
  // if overflow occurs, carry one
  for (i = iter->len-1; i >= 0; --i) {
    if (iter->array[i] < iter->lengths[i]) {
      iter->array[i]++;
      return 1;
    } else {
      iter->array[i] = 1;
    }
  }
  
  return 0;
}

void iter_destroy(struct permut_iter *iter)
{
  free(iter->array);
  free(iter->lengths);
  free(iter);
}