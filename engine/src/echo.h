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

#ifndef flowy_engine_echo_h
#define flowy_engine_echo_h

#include "pipeline.h"
#include "base.h"
#include "ftreader.h"
#include "ftwriter.h"

/* -----------------------------------------------------------------------*/
/*                              branch                                    */
/* -----------------------------------------------------------------------*/

void
echo_branch(size_t num_branches,
            struct branch** branchset,
            struct ft_data* trace);

void
echo_filter(
            const int const branch_id,
            const struct filter_result* const fresult,
            struct ft_data* const dataformat
           );

void
echo_grouper(
             const int const branch_id,
             size_t num_grouper_rules,
             size_t num_sorted_records,

             const struct grouper_result* const gresult,
             struct ft_data* const dataformat
            );


void
echo_group_aggr(
                const int const branch_id,
                const struct grouper_result* const gresult,
                struct ft_data* const dataformat
               );

void
echo_gfilter(
             const int const branch_id,
             const struct groupfilter_result* const gfresult,
             struct ft_data* const dataformat
            );

/* -----------------------------------------------------------------------*/


void
echo_merger(
            size_t num_branches,
            struct branch** const branchset,

            const struct merger_result* const mresult,
            struct ft_data* const dataformat
            );


void
echo_results(
             const struct ungrouper_result* const uresult,
             struct ft_data* const dataformat
            );


#endif
