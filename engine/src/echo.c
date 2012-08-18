
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

#include "echo.h"


void
echo_merger(
            size_t num_branches,
            struct branch** const branchset,

            const struct merger_result* const mresult,
            struct ft_data* const dataformat
           ) {
#ifdef FOO  

  if (verbose_vv) {

    if(!file) {

      struct permut_iter* iter = iter_init(num_branches, branchset);

      printf("\nNo. of (to be) Matched Groups: %zu \n",
             mresult->total_num_group_tuples);
      if (iter != NULL) {
        puts(FLOWHEADER);
        while(iter_next(iter)) {
          for (int j = 0; j < num_branches; j++) {
            char* record = branchset[j]->gfilter_result->filtered_groupset
                              [
                               iter->filtered_group_tuple[j] - 1
                              ]->aggr_result->aggr_record;
            flow_print_record(dataformat, record);
          }
          printf("\n");
        }
        iter_destroy(iter);
      }
    }
  }
#endif
  
  if(!file) {

    printf("\nNo. of Merged Groups: %u (Tuples)\n", mresult->num_matches);
    if (mresult->num_matches != 0)
      puts(FLOWHEADER);

    for (int j = 0; j < mresult->num_matches; j++) {
      struct merger_match* match = mresult->matchset[j];
      for (int k = 0; k < match->num_groups; k++) {
        struct group* group = match->groupset[k];
        flow_print_record(dataformat, group->aggr_result->aggr_record);
      }
      printf("\n");
    }
  }
}



/* -----------------------------------------------------------------------*/
/*                              branch                                    */
/* -----------------------------------------------------------------------*/

void
echo_branch(
            size_t num_branches,
            struct branch** branchset,
            struct ft_data* dataformat
           ){


  /* process each branch */
  for (int i = 0; i < num_branches; i++) {

    struct branch* branch = branchset[i];
#ifdef GROUPER
    if (grouper_enabled){
      if (verbose_vv) {

        echo_grouper(
                      branch->branch_id,
                      branch->num_grouper_clauses,
                      branch->filter_result->num_filtered_records,

                      branch->grouper_result,
                      branch->data
                    );
      }
    }
#endif


#ifdef GROUPERAGGREGATIONS
    if (grouper_enabled) {

      echo_group_aggr(
                      branch->branch_id,
                      branch->grouper_result,
                      branch->data
                      );
    }
#endif


#ifdef GROUPFILTER
    if (groupfilter_enabled) {

      echo_gfilter(
                    branch->branch_id,
                    branch->gfilter_result,
                    branch->data
                  );
    }
#endif
  }
}

void
echo_filter(
            int branch_id,
            const struct filter_result* const fresult,
            struct ft_data* const dataformat
           ) {

  if(!file) {

    printf("\nNo. of Filtered Records: %u\n", fresult->num_filtered_records);
    if (fresult->num_filtered_records != 0)
      puts(FLOWHEADER);

    for (int j = 0; j < fresult->num_filtered_records; j++) {
      char* record = fresult->filtered_recordset[j];
      flow_print_record(dataformat, record);
    }
  }
}

void
echo_grouper(
             int branch_id,
             size_t num_grouper_clauses,
             uint32_t num_sorted_records,

             const struct grouper_result* const gresult,
             struct ft_data* const dataformat
            ) {

  if(num_grouper_clauses > 0) {

    /* sorted records */
    if (!file) {

      printf("\nNo. of Sorted Records: %u\n", num_sorted_records);
      if (num_sorted_records != 0)
        puts(FLOWHEADER);

      for (int j = 0; j < num_sorted_records; j++)
        flow_print_record(dataformat, gresult->sorted_recordset[j]);
    }
  }

  /* group members */
  if(!file) {
    printf("\nNo. of Groups: %u (Verbose Output)\n", gresult->num_groups);

    if (gresult->num_groups > 0)
      puts(FLOWHEADER);
  }
  for (int j = 0; j < gresult->num_groups; j++) {

    struct group* group = gresult->groupset[j];

    if (!file) {
      printf("\n");
      /* print group members */
      for (int k = 0; k < group->num_members; k++)
        flow_print_record(dataformat, group->members[k]);
    }
  }
}

void
echo_group_aggr(
                int branch_id,
                const struct grouper_result* const gresult,
                struct ft_data* const dataformat
               ) {

  /* write to the file is directly done by the thread */
  if(!file) {

    printf("\nNo. of Groups: %u (Aggregations)\n", gresult->num_groups);
    if (gresult->num_groups != 0)
      puts(FLOWHEADER);
    for (int j = 0; j < gresult->num_groups; j++) {
      struct group* group = gresult->groupset[j];
      flow_print_record(dataformat, group->aggr_result->aggr_record);
    }
  }
}

void
echo_gfilter(
             int branch_id,
             const struct groupfilter_result* const gfresult,
             struct ft_data* const dataformat
            ) {

  if(!file) {

    printf("\nNo. of Filtered Groups: %u (Aggregations)\n",
           gfresult->num_filtered_groups);
    if (gfresult->num_filtered_groups != 0)
      puts(FLOWHEADER);

    for (int j = 0; j < gfresult->num_filtered_groups; j++) {
      struct group* fgroup = gfresult->filtered_groupset[j];
      flow_print_record(dataformat, fgroup->aggr_result->aggr_record);
    }
  }
}

/* -----------------------------------------------------------------------*/

void
echo_results(
             const struct ungrouper_result* const uresult,
             struct ft_data* const dataformat
            ) {


  /* -----------------------------------------------------------------------*/
  /*                                results                                 */
  /* -----------------------------------------------------------------------*/

  if(!file) {
    printf("\nNo. of Streams: %zu \n", uresult->num_streams);
    printf("----------------- \n");

    for (int j = 0; j < uresult->num_streams; j++) {

      struct stream* stream = uresult->streamset[j];

      if(!file) {

        printf("\nNo. of Records in Stream (%d): %u \n",j+1,
               stream->num_records);
        if (stream->num_records != 0)
          puts(FLOWHEADER);

        for (int i = 0; i < stream->num_records; i++) {
          char* record = stream->recordset[i];
          flow_print_record(dataformat, record);
        }

        printf("\n");
      }
    }
  }

  /* -----------------------------------------------------------------------*/

}
