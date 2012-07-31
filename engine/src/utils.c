
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
/*                         query parsing utilities                        */
/* -----------------------------------------------------------------------*/

size_t
get_offset(const char * const name,
           const struct fts3rec_offsets* const offsets) {

  #define CASEOFF(memb)                       \
  if (strcmp(name, #memb) == 0)               \
    return offsets->memb

	CASEOFF(unix_secs);
	CASEOFF(unix_nsecs);
	CASEOFF(sysUpTime);
	CASEOFF(exaddr);
	CASEOFF(srcaddr);
	CASEOFF(dstaddr);
	CASEOFF(nexthop);
	CASEOFF(input);
	CASEOFF(output);
	CASEOFF(dFlows);
	CASEOFF(dPkts);
	CASEOFF(dOctets);
	CASEOFF(First);
	CASEOFF(Last);
	CASEOFF(srcport);
	CASEOFF(dstport);
	CASEOFF(prot);
	CASEOFF(tos);
	CASEOFF(tcp_flags);
	CASEOFF(pad);
	CASEOFF(engine_type);
	CASEOFF(engine_id);
	CASEOFF(src_mask);
	CASEOFF(dst_mask);
	CASEOFF(src_as);
	CASEOFF(dst_as);
	CASEOFF(in_encaps);
	CASEOFF(out_encaps);
	CASEOFF(peer_nexthop);
	CASEOFF(router_sc);
	CASEOFF(src_tag);
	CASEOFF(dst_tag);
	CASEOFF(extra_pkts);
	CASEOFF(marked_tos);

  return -1;
}



uint64_t
get_enum(const char * const name) {

  #define CASEENUM(memb)                      \
  if (strcmp(name, #memb) == 0)               \
    return memb

  CASEENUM(RULE_S2_8);
  CASEENUM(RULE_S2_16);
  CASEENUM(RULE_S2_32);
  CASEENUM(RULE_S2_64);
  CASEENUM(RULE_S1_8);
  CASEENUM(RULE_S1_16);
  CASEENUM(RULE_S1_32);
  CASEENUM(RULE_S1_64);
  CASEENUM(RULE_ABS);
  CASEENUM(RULE_REL);
  CASEENUM(RULE_NO);
  CASEENUM(RULE_EQ);
  CASEENUM(RULE_NE);
  CASEENUM(RULE_GT);
  CASEENUM(RULE_LT);
  CASEENUM(RULE_LE);
  CASEENUM(RULE_GE);
  CASEENUM(RULE_STATIC);
  CASEENUM(RULE_COUNT);
  CASEENUM(RULE_UNION);
  CASEENUM(RULE_MIN);
  CASEENUM(RULE_MAX);
  CASEENUM(RULE_MEDIAN);
  CASEENUM(RULE_MEAN);
  CASEENUM(RULE_STDDEV);
  CASEENUM(RULE_XOR);
  CASEENUM(RULE_SUM);
  CASEENUM(RULE_PROD);
  CASEENUM(RULE_AND);
  CASEENUM(RULE_OR);
  CASEENUM(RULE_IN);
  CASEENUM(RULE_ALLEN_BF);
  CASEENUM(RULE_ALLEN_AF);
  CASEENUM(RULE_ALLEN_M);
  CASEENUM(RULE_ALLEN_MI);
  CASEENUM(RULE_ALLEN_O);
  CASEENUM(RULE_ALLEN_OI);
  CASEENUM(RULE_ALLEN_S);
  CASEENUM(RULE_ALLEN_SI);
  CASEENUM(RULE_ALLEN_D);
  CASEENUM(RULE_ALLEN_DI);
  CASEENUM(RULE_ALLEN_F);
  CASEENUM(RULE_ALLEN_FI);
  CASEENUM(RULE_ALLEN_EQ);

  return -1;
}



/* -----------------------------------------------------------------------*/











/* -----------------------------------------------------------------------*/
/*                         grouper utilities                              */
/* -----------------------------------------------------------------------*/

/* grouper qsort comparator functions */
#if defined(__APPLE__) || defined(__FreeBSD__)
  #define comp(size) \
    int \
    comp_##size(void *thunk, const void *e1, const void *e2) {\
      size x, y; \
      x = *(size *)(**(char ***)e1 + (size_t)thunk); \
      y = *(size *)(**(char ***)e2 + (size_t)thunk); \
      return ((x > y) - (y > x)); \
    }
#elif defined(__linux)
  #define comp(size) \
    int \
    comp_##size(const void *e1, const void *e2, void *thunk) {\
      size x, y; \
      x = *(size *)(**(char ***)e1 + (size_t)thunk); \
      y = *(size *)(**(char ***)e2 + (size_t)thunk); \
      return ((x > y) - (y > x)); \
    }
#endif
comp(uint8_t);
comp(uint16_t);
comp(uint32_t);
comp(uint64_t);


/*
 * adapted from ./stdlib/bsearch.c from the GNU C Library
 * pass additional argument thunk to compar to allow it access additional data
 * without global variables - in our case, we need to pass the data offset */
void *
bsearch_r (
           const void *key,
           const void *base,
           size_t nmemb,
           size_t size,
           void *thunk,
           int (*compar) (void *thunk, const void *, const void *),
           enum bsearch_item get_item
          ) {
  
  size_t l, u, idx;
  const void *p;
  void* item;
  int comparison;
  
  l = 0;
  u = nmemb - 1;
  while (l <= u) {
    
    if (get_item == LAST_ITEM)
      idx = (size_t) ceil((l + u) / 2.0);
    else
      idx = (size_t) floor((l + u) / 2.0);

    if (get_item == RANDOM_ITEM) {
      item = (((char ****) base) + (idx * size));
      p = (void *) ***(char ****)(item);
    }
    else {
      item = (((char ***) base) + (idx * size));
      p = (void *) **(char ***)(item);
    }
    if (p != NULL) {
      comparison = (*compar) (thunk, key, p);
    }else {
      /* the record was seen before is now NULL, must be less than this one */
      comparison = 1;
    }
    
    if (comparison < 0)
      u = idx - 1;
    else if (comparison > 0)
      l = idx + 1;
    else if (get_item == FIRST_ITEM && l!= idx)
      u = idx;
    else if (get_item == LAST_ITEM && u!= idx)
      l = idx;
    else {
      return (void *) item;
    }
  }
  return NULL;
}



#define comp_pp(size) \
int comp_##size##_pp(void *thunk, const void *e1, const void *e2) {\
size x, y; \
x = *(size *)((char *)e1 + *(size_t *)thunk); \
y = *(size *)((char *)e2 + *(size_t *)thunk); \
return (x > y) - (y > x); \
}

comp_pp(uint8_t);
comp_pp(uint16_t);
comp_pp(uint32_t);
comp_pp(uint64_t);


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
struct permut_iter*
iter_init(
          size_t num_branches,
          struct branch** const branchset
         ) {

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
    branchset[i]->gfilter_result->num_filtered_groups;

    /* break out if any branch has no filtered groups */
    if (iter->num_filtered_groups[i] == 0) {
      iter_destroy(iter);
      return NULL;
    }
  }

  /* the first call to iter_next will switch it to 1 */
  iter->filtered_group_tuple[num_branches-1] = 0;
  return iter;
}

/*
 * given the iterator, modify to represent the next permutation
 * and return true. if the last permutation is reached, return false.
 */
bool iter_next(const struct permut_iter *iter) {

  /* start from right to left in the group tuple */
  for (int i = (int) iter->num_branches-1; i >= 0; --i) {

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
