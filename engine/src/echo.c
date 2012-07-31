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

  if (verbose_vv) {

    struct permut_iter* iter = iter_init(num_branches, branchset);

    if(!file) {
      printf("\nNo. of (to be) Matched Groups: %zu \n",
             mresult->total_num_group_tuples);
    }

    if (iter != NULL) {
      if (file) {
        /* get a file descriptor */
        char* filename = (char*)0L;
        asprintf(&filename, "%s/merger-to-be-merged-groups.ftz", dirpath);
        int out_fd = get_fd(filename);
        if(out_fd == -1) errExit("get_fd(...) returned -1");
        else free(filename);
        
        uint32_t num_flows =
        (uint32_t) (mresult->total_num_group_tuples * num_branches);
        
        /* get the output stream */
        struct ftio* ftio_out = get_ftio(
                                         dataformat,
                                         out_fd,
                                         num_flows
                                        );

        /* write the header to the output stream */
        int n = -1;
        if ((n = ftio_write_header(ftio_out)) < 0)
          fterr_errx(1, "ftio_write_header(): failed");

        while(iter_next(iter)) {
          for (int j = 0; j < num_branches; j++) {
            /* write the record to the output stream */
            char* record = branchset[j]->gfilter_result->filtered_groupset
                              [
                               iter->filtered_group_tuple[j] - 1
                              ]->aggr_result->aggr_record;
            if ((n = ftio_write(ftio_out, record) < 0))
              fterr_errx(1, "ftio_write(): failed");
          }
        }

        iter_destroy(iter);

        /* close the output stream */
        if ((n = ftio_close(ftio_out)) < 0)
          fterr_errx(1, "ftio_close(): failed");
        free(ftio_out);

      } else {

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

  if(file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/merger-merged-groups.ftz", dirpath);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);
    
    uint32_t num_flows = (uint32_t) (mresult->num_group_tuples * num_branches);

    /* get the output stream */
    struct ftio* ftio_out = get_ftio(
                                     dataformat,
                                     out_fd,
                                     num_flows
                                    );

    /* write the header to the output stream */
    int n = -1;
    if ((n = ftio_write_header(ftio_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");


    for (int j = 0; j < mresult->num_group_tuples; j++) {
      /* write the record to the output stream */
      struct group** group_tuple = mresult->group_tuples[j];
      for (int i = 0; i < num_branches; i++) {
        struct group* group = group_tuple[i];
        if ((n = ftio_write(ftio_out, group->aggr_result->aggr_record) < 0))
          fterr_errx(1, "ftio_write(): failed");
      }
    }

    /* close the output stream */
    if ((n = ftio_close(ftio_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_out);
  } else {

    printf("\nNo. of Merged Groups: %u (Tuples)\n", mresult->num_group_tuples);
    if (mresult->num_group_tuples != 0)
      puts(FLOWHEADER);

    for (int j = 0; j < mresult->num_group_tuples; j++) {
      struct group** group_tuple = mresult->group_tuples[j];
      for (int i = 0; i < num_branches; i++) {
        struct group* group = group_tuple[i];
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

#ifdef GROUPER
    struct branch* branch = branchset[i];
    if (verbose_vv){
      echo_grouper(
                   branch->branch_id,
                   branch->num_grouper_clauses,
                   branch->filter_result->num_filtered_records,

                   branch->grouper_result,
                   branch->data
                  );
    }
#endif


#ifdef GROUPERAGGREGATIONS
    echo_group_aggr(
                    branch->branch_id,
                    branch->grouper_result,
                    branch->data
                   );
#endif


#ifdef GROUPFILTER
    echo_gfilter(
                 branch->branch_id,
                 branch->gfilter_result,
                 branch->data
                );
#endif
  }
}

void
echo_filter(
            int branch_id,
            const struct filter_result* const fresult,
            struct ft_data* const dataformat
           ) {

  if(file) {
    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/filter-branch-%d-filtered-records.ftz", 
             dirpath, branch_id);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);

    /* get the output stream */
    struct ftio* ftio_out = get_ftio(
                                     dataformat,
                                     out_fd,
                                     fresult->num_filtered_records
                                    );
    /* write the header to the output stream */
    int n = -1;
    if ((n = ftio_write_header(ftio_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");


    for (int j = 0; j < fresult->num_filtered_records; j++) {

      char* record = fresult->filtered_recordset[j];
      /* write the record to the output stream */
      if ((n = ftio_write(ftio_out, record)) < 0)
        fterr_errx(1, "ftio_write(): failed");
    }

    /* close the output stream */
    if ((n = ftio_close(ftio_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_out);

  } else {
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

    if (file) {
      /* get a file descriptor */
      char* filename = (char*)0L;
      asprintf(&filename, "%s/grouper-branch-%d-sorted-records.ftz", 
               dirpath, branch_id);
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      struct ftio* ftio_out = get_ftio(
                                       dataformat,
                                       out_fd,
                                       num_sorted_records
                                      );

      /* write the header to the output stream */
      int n = -1;
      if ((n = ftio_write_header(ftio_out)) < 0)
        fterr_errx(1, "ftio_write_header(): failed");

      for (int j = 0; j < num_sorted_records; j++) {
        /* write the record to the output stream */
        if ((n = ftio_write(ftio_out, gresult->sorted_recordset[j])) < 0)
          fterr_errx(1, "ftio_write(): failed");
      }

      /* close the output stream */
      if ((n = ftio_close(ftio_out)) < 0)
        fterr_errx(1, "ftio_close(): failed");
      free(ftio_out);

    } else {

      printf("\nNo. of Sorted Records: %u\n", num_sorted_records);
      if (num_sorted_records != 0)
        puts(FLOWHEADER);

      for (int j = 0; j < num_sorted_records; j++)
        flow_print_record(dataformat, gresult->sorted_recordset[j]);
    }

    if (file) {
      /* get a file descriptor */
      char* filename = (char*)0L;
      asprintf(&filename, "%s/grouper-branch-%d-unique-records.ftz",
               dirpath, branch_id);
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      struct ftio* ftio_out = get_ftio(
                                        dataformat,
                                        out_fd,
                                        gresult->num_unique_records
                                      );

      /* write the header to the output stream */
      int n = -1;
      if ((n = ftio_write_header(ftio_out)) < 0)
        fterr_errx(1, "ftio_write_header(): failed");

      for (int j = 0; j < gresult->num_unique_records; j++) {
        /* write the record to the output stream */
        if ((n = ftio_write(ftio_out, gresult->unique_recordset[j])) < 0)
          fterr_errx(1, "ftio_write(): failed");
      }

      /* close the output stream */
      if ((n = ftio_close(ftio_out)) < 0)
        fterr_errx(1, "ftio_close(): failed");
      free(ftio_out);

    } else {
      printf("\nNo. of Unique Records: %u\n", gresult->num_unique_records);
      if (gresult->num_unique_records != 0)
        puts(FLOWHEADER);

      for (int j = 0; j < gresult->num_unique_records; j++)
        flow_print_record(dataformat, gresult->unique_recordset[j]);
    }
  }

  if(!file) {
    printf("\nNo. of Groups: %u (Verbose Output)\n", gresult->num_groups);

    if (gresult->num_groups > 0)
      puts(FLOWHEADER);
  }

  for (int j = 0; j < gresult->num_groups; j++) {

    struct group* group = gresult->groupset[j];

    if (file) {
      /* get a file descriptor */
      char* filename = (char*)0L;
      asprintf(&filename, "%s/grouper-branch-%d-group-%d-records.ftz",
               dirpath, branch_id, j);
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      struct ftio* ftio_out = get_ftio(
                                       dataformat,
                                       out_fd,
                                       group->num_members
                                      );

      /* write the header to the output stream */
      int n = -1;
      if ((n = ftio_write_header(ftio_out)) < 0)
        fterr_errx(1, "ftio_write_header(): failed");

      /* print group members */
      for (int k = 0; k < group->num_members; k++) {

        /* write the record to the output stream */
        if ((n = ftio_write(ftio_out, group->members[k]) < 0))
          fterr_errx(1, "ftio_write(): failed");
      }

      /* close the output stream */
      if ((n = ftio_close(ftio_out)) < 0)
        fterr_errx(1, "ftio_close(): failed");
      free(ftio_out);

    } else {
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

  if(file) {
    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/grouper-branch-%d-groups.ftz",
             dirpath, branch_id);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);

    /* get the output stream */
    struct ftio* ftio_out = get_ftio(
                                     dataformat,
                                     out_fd,
                                     gresult->num_groups
                                    );

    /* write the header to the output stream */
    int n = -1;
    if ((n = ftio_write_header(ftio_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");

    for (int j = 0; j < gresult->num_groups; j++) {
      /* write the record to the output stream */
      struct group* group = gresult->groupset[j];
      if ((n = ftio_write(ftio_out, group->aggr_result->aggr_record)) < 0)
        fterr_errx(1, "ftio_write(): failed");
    }

    /* close the output stream */
    if ((n = ftio_close(ftio_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_out);
  } else{

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

  if(file) {

    /* get a file descriptor */
    char* filename = (char*)0L;
    asprintf(&filename, "%s/groupfilter-branch-%d-filtered-groups.ftz",
             dirpath, branch_id);
    int out_fd = get_fd(filename);
    if(out_fd == -1) errExit("get_fd(...) returned -1");
    else free(filename);

    /* get the output stream */
    struct ftio* ftio_out = get_ftio(
                                     dataformat,
                                     out_fd,
                                     gfresult->num_filtered_groups
                                    );

    /* write the header to the output stream */
    int n = -1;
    if ((n = ftio_write_header(ftio_out)) < 0)
      fterr_errx(1, "ftio_write_header(): failed");

    for (int j = 0; j < gfresult->num_filtered_groups; j++) {

      /* write the record to the output stream */
      struct group* fgroup = gfresult->filtered_groupset[j];
      if ((n = ftio_write(ftio_out, fgroup->aggr_result->aggr_record)) < 0)
        fterr_errx(1, "ftio_write(): failed");
    }

    /* close the output stream */
    if ((n = ftio_close(ftio_out)) < 0)
      fterr_errx(1, "ftio_close(): failed");
    free(ftio_out);
  } else {

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
  }

  for (int j = 0; j < uresult->num_streams; j++) {

    struct stream* stream = uresult->streamset[j];

    if(file) {

      /* get a file descriptor */
      char* filename = (char*)0L;
      asprintf(&filename, "%s/ungrouper-stream-%d.ftz",dirpath,j);
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      struct ftio* ftio_out = get_ftio(
                                       dataformat,
                                       out_fd,
                                       stream->num_records
                                      );

      /* write the header to the output stream */
      int n = -1;
      if ((n = ftio_write_header(ftio_out)) < 0)
        fterr_errx(1, "ftio_write_header(): failed");

      for (int i = 0; i < stream->num_records; i++) {

        /* write the record to the output stream */
        char* record = stream->recordset[i];
        if ((n = ftio_write(ftio_out, record)) < 0)
          fterr_errx(1, "ftio_write(): failed");
      }

      /* close the output stream */
      if ((n = ftio_close(ftio_out)) < 0)
        fterr_errx(1, "ftio_close(): failed");
      free(ftio_out);

    } else {

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

  /* -----------------------------------------------------------------------*/

}
