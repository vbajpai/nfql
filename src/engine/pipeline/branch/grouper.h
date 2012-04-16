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

#ifndef flowy_engine_grouper_fptr_h
#define flowy_engine_grouper_fptr_h

#include "pipeline.h"
#include "utils.h"
#include "error_handlers.h"
#include "ftreader.h"
#include "auto_assign.h"

struct grouper_rule;
struct grouper_aggr;

#define tree_item(size) \
struct tree_item_##size { \
size                            value; \
char***                         ptr; \
};

tree_item(uint8_t);
tree_item(uint16_t);
tree_item(uint32_t);
tree_item(uint64_t);

typedef enum { UINT8_T, UINT16_T, UINT32_T, UINT64_T } int_sizes;

struct grouper_intermediate_result {    
  
  size_t                          num_uniq_records;
  
  union {
    struct tree_item_uint8_t*     recordset_uint8_t;
    struct tree_item_uint16_t*    recordset_uint16_t;
    struct tree_item_uint32_t*    recordset_uint32_t;
    struct tree_item_uint64_t*    recordset_uint64_t;
  }uniq_recordset;
  
  char***                         sorted_recordset_reference;
};

struct grouper_type {
  
  int (*qsort_comp)(
                    void*                           thunk,
                    const void*                     e1,
                    const void*                     e2
                    );
  
  char*** (*bsearch)(
                     const char* const              filtered_record,
                     struct grouper_rule** const    grouper_ruleset,
                     const struct 
                     grouper_intermediate_result* 
                     const                          intermediate_result
                     );  
};


struct aggr_result*
grouper_aggregations(
                     size_t num_filter_rules,
                     struct filter_rule** const filter_ruleset,
                     
                     size_t num_grouper_rules,
                     struct grouper_rule** const grouper_ruleset,
                     
                     size_t num_aggr_rules,
                     struct aggr_rule** const aggr_ruleset,
                     
                     const struct group* const group,
                     int rec_size
                     );

struct grouper_intermediate_result *
get_grouper_intermediates(
                          size_t num_filtered_records,
                          char** const filtered_recordset_copy,
                          
                          size_t num_grouper_rules,
                          struct grouper_rule** const grouper_ruleset,
                          
                          struct grouper_result* const gresult,
                          const struct grouper_type* const gtype
                          );

struct grouper_result*
grouper(
        size_t num_filter_rules,
        struct filter_rule** const filter_ruleset,
        
        size_t num_grouper_rules,
        struct grouper_rule** const grouper_ruleset,
        
        size_t num_aggr_rules,
        struct aggr_rule** const aggr_ruleset,
        
        const struct filter_result* const fresult,
        int rec_size        
        );


int 
comp_uint8_t(void *thunk, const void *e1, const void *e2);

int 
comp_uint16_t(void *thunk, const void *e1, const void *e2);

int 
comp_uint32_t(void *thunk, const void *e1, const void *e2);

int 
comp_uint64_t(void *thunk, const void *e1, const void *e2);

int 
comp_uint8_t_p(void *thunk, const void *e1, const void *e2);

int 
comp_uint16_t_p(void *thunk, const void *e1, const void *e2);

int 
comp_uint32_t_p(void *thunk, const void *e1, const void *e2);

int 
comp_uint64_t_p(void *thunk, const void *e1, const void *e2);

#endif
