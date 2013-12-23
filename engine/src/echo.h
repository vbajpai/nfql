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

#ifndef f_engine_echo_h
#define f_engine_echo_h

#include "pipeline.h"
#include "base.h"
#include "merger.h"
#include "io.h"

/* -----------------------------------------------------------------------*/
/*                              branch                                    */
/* -----------------------------------------------------------------------*/

void
echo_branch(size_t num_branches,
            struct branch** branchset,
            struct io_handler_s* io_handler,
            struct io_reader_s* read_ctxt);

void
echo_filter(
            const int branch_id,
            const struct filter_result* fresult,
            struct io_handler_s* io,
            struct io_reader_s* read_ctxt
           );

void
echo_grouper(
             const int branch_id,
             size_t num_grouper_rules,
             uint32_t num_sorted_records,

             const struct grouper_result* gresult,
             struct io_handler_s* io,
             struct io_reader_s* read_ctxt
            );


void
echo_group_aggr(
                const int branch_id,
                const struct grouper_result* gresult,
                struct io_handler_s* io,
                struct io_reader_s* read_ctxt
               );

void
echo_gfilter(
             const int branch_id,
             const struct groupfilter_result* gfresult,
             struct io_handler_s* io,
             struct io_reader_s* read_ctxt
            );

/* -----------------------------------------------------------------------*/


void
echo_merger(
            size_t num_branches,
            struct branch** const branchset,

            const struct merger_result* mresult,
            struct io_handler_s* io,
            struct io_reader_s* read_ctxt
            );


void
echo_results(
             const struct ungrouper_result* uresult,
             struct io_handler_s* io,
             struct io_reader_s* read_ctxt
            );


#endif
