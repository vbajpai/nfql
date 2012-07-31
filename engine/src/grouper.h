/*
 * Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
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

#ifndef f_engine_grouper_h
#define f_engine_grouper_h

#include "pipeline.h"
#include "utils.h"
#include "errorhandlers.h"
#include "ftreader.h"
#include "auto-assign.h"

struct grouper_term;
struct grouper_aggr;

struct grouper_intermediate_result {

  struct uniq_recordset_result*   uniq_result;
  char***                         sorted_recordset_reference;
};

struct uniq_recordset_result {

  uint32_t                        num_uniq_records;
  char****                        uniq_recordset;
};


/* quick sort comparator */
#if defined(__APPLE__) || defined(__FreeBSD__)
  int qsort_comp(void *thunk, const void *e1, const void *e2);
#elif defined(__linux)
  int qsort_comp(const void *e1, const void *e2, void *thunk);
#endif


struct grouper_type {
  
  #if defined (__APPLE__) || defined (__FreeBSD__)
  int (*qsort_comp)(
                      void*                           thunk,
                      const void*                     e1,
                      const void*                     e2
                   );
  #elif defined (__linux)
  int (*qsort_comp)(
                      const void*                     e1,
                      const void*                     e2,
                      void*                           thunk
                   );
  #endif

  struct search_result*
          (*bsearch)(
                      const char* const              key,
                      const void* const              base,
                      
                      size_t                         num_filtered_records,
                      size_t                         field_offset,
                      int                            type
                    );

  struct uniq_recordset_result*
  (*alloc_uniqresult)
            (
              size_t                         num_filtered_records,
              struct grouper_term** const    grouper_termset_of_first_clause,
              char*** const                  sorted_recordset_ref
            );

  char*
  (*get_uniq_record)(
                     const struct uniq_recordset_result* const uniq_result,
                     int index
                    );

  void
  (*dealloc_uniqresult)(struct uniq_recordset_result* uniq_result);
};

struct search_result {
  char***                         record_iter;
  size_t                          num_items;
};


char***
grouper_bsearch (
                 const char* filtered_record,
                 struct grouper_clause* clause,
                 const struct grouper_intermediate_result* const uniq_result,
                 size_t num_filtered_records
                );


struct aggr_result*
grouper_aggregations(
                     size_t num_filter_clauses,
                     struct filter_clause** const filter_clauseset,

                     size_t num_grouper_clauses,
                     struct grouper_clause** const grouper_clauseset,

                     size_t num_aggr_clause_terms,
                     struct aggr_term** const aggr_clause_termset,

                     const struct group* const group,
                     int rec_size
                     );

struct grouper_intermediate_result *
get_grouper_intermediates
                (
                  size_t num_filtered_records,
                  char** const filtered_recordset_copy,
                 
                  struct grouper_clause* clause,
                  struct grouper_result* const gresult
                );

struct grouper_result*
grouper(
        size_t num_filter_clauses,
        struct filter_clause** const filter_clauseset,

        size_t num_grouper_clauses,
        struct grouper_clause** const grouper_clauseset,

        size_t num_aggr_clause_terms,
        struct aggr_term** const aggr_clause_termset,

        const struct filter_result* const fresult,
        int rec_size
       );

#endif
