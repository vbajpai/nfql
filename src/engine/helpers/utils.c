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


/* -----------------------------------------------------------------------*/  
/*                         grouper utilities                              */
/* -----------------------------------------------------------------------*/

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

/* -----------------------------------------------------------------------*/  











/* -----------------------------------------------------------------------*/  
/*                         merger utilities                               */
/* -----------------------------------------------------------------------*/

/*
 * Initialize a permutation iterator. 
 * Iterator goes through all possible permutations of filtered group tuples
 * which are used to make a match. This technique is an alternative to using
 * deep nested loops which cannot be used in C since the level of nesting
 * depends on the number of branches which is evaluation when the query is 
 * parsed at RUNTIME
 */
struct permut_iter *
iter_init(struct branch_info* branchset, 
          int num_branches) {
  
  /* free'd using iter_destroy(...) called before returning from merger(...) */
  struct permut_iter *iter = (struct permut_iter *)
                              calloc(1, sizeof(struct permut_iter));
  if (iter == NULL)
    errExit("calloc");    
  iter->num_branches = num_branches;
  
  /* free'd using iter_destroy(...) called before returning from merger(...) */
  iter->filtered_group_tuple = (size_t *)calloc(num_branches, sizeof(size_t));  
  if (iter->filtered_group_tuple == NULL)
    errExit("calloc");

  /* free'd using iter_destroy(...) called before returning from merger(...) */
  iter->num_filtered_groups = (size_t *)calloc(num_branches, sizeof(size_t));  
  if (iter->num_filtered_groups == NULL)
    errExit("calloc");
  
  /* the first group tuple is (g1_b1, g1_b2, ...) */
  for (int i = 0; i < num_branches; i++) {
    iter->filtered_group_tuple[i] = 1;
    iter->num_filtered_groups[i] = 
    branchset[i].gfilter_result->num_filtered_groups;
  }
  
  /* the first call to iter_next will switch it to 1 */
  iter->filtered_group_tuple[num_branches-1] = 0;
  return iter;
}

/*
 * given the iterator, modify to represent the next permutation
 * and return true. if the last permutation is reached, return false.
 */
bool iter_next(struct permut_iter *iter) {
  
  /* start from right to left in the group tuple */
  for (int i = iter->num_branches-1; i >= 0; --i) {
    
    /* if the item in particular index of the tuple has more left elements,
     * choose the next element, and return
     */
    if (iter->filtered_group_tuple[i] < iter->num_filtered_groups[i]) {
      iter->filtered_group_tuple[i]++;
      return true;
    /* if all the elements in this particular index of the tuple have been
       checked, wrap the group around, and move to another (left) index      
     */  
    } else {
      iter->filtered_group_tuple[i] = 1;
    }
  }
  
  /* if everything is matched, return false */
  return false;
}

void iter_destroy(struct permut_iter *iter) {  
  free(iter->filtered_group_tuple); iter->filtered_group_tuple = NULL;
  free(iter->num_filtered_groups); iter->num_filtered_groups = NULL;
  free(iter); iter = NULL;
}

/* -----------------------------------------------------------------------*/