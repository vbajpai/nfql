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

#define _GNU_SOURCE

#include "grouper.h"

#include "io.h"


struct aggr_result*
grouper_aggregations(
                     size_t num_filter_clauses,
                     struct filter_clause** const filter_clauseset,

                     size_t num_grouper_clauses,
                     struct grouper_clause** const grouper_clauseset,

                     size_t num_aggr_clause_terms,
                     struct aggr_term** const aggr_clause_termset,

                     const struct group* const group,
                     struct aggr_result* aresult
                     ) {

  /* free'd just after returning from merger(...) */
  struct aggr** aggrset = (struct aggr**)
  calloc(num_aggr_clause_terms,
         sizeof(struct aggr*));
  if (aggrset == NULL)
    errExit("calloc");
  aresult->aggrset = aggrset;

  /* aggregate the fields, but ignore fields used in grouper and filter
   * module. Again it assume that the operation is `eq`. Need to investigate
   * more on how it might affect for other type of operations */
  for (int j = 0; j < num_aggr_clause_terms; j++){

    bool if_ignore_aggr_term = false;
    struct aggr_term* term = aggr_clause_termset[j];
    size_t aggr_offset = term->field_offset;


    /* if aggr rule is same as any filter rule, just ignore it */
    for (int i = 0; i < num_filter_clauses; i++) {

      struct filter_clause* fclause = filter_clauseset[i];
      for (int k = 0; k < fclause->num_terms; k++) {

        size_t filter_offset = fclause->termset[k]->field_offset;
        if (aggr_offset == filter_offset){
          if_ignore_aggr_term = true;
          break;
        }
      }
    }

    /* if aggr rule is same as any grouper rule, just ignore it */
    for (int i = 0; i < num_grouper_clauses; i++) {

      struct grouper_clause* gclause = grouper_clauseset[i];

      for (int k = 0; k < gclause->num_terms; k++) {

        struct grouper_term* term = gclause->termset[k];

        size_t goffset_1 = term->field_offset1;
        size_t goffset_2 = term->field_offset2;

        if (aggr_offset == goffset_1 || aggr_offset == goffset_2){
          if_ignore_aggr_term = true;
          break;
        }
      }
    }

    /* free'd just after returning from merger(...) */
    aggrset[j] = term->func(
                             group->members,
                             aresult->aggr_record->aggr_record,
                             group->num_members,
                             aggr_offset,
                             if_ignore_aggr_term
                           );
  }
  return aresult;
}

char**
get_grouper_intermediates
                (
                  uint32_t num_filtered_records,
                  char** const filtered_recordset,

                  struct grouper_clause* clause,
                  struct grouper_result* const gresult,

                  struct io_handler_s* io,
                  struct io_reader_s*  read_ctxt,
                  int branch_id
                ) {

  /* sort the record references according to the right hand side
   * item in the statement of the first grouper term of the first clause
   * and save them in sorted_recordset_reference in place */
  #if defined(__APPLE__) || defined(__FreeBSD__)
    qsort_r(
             filtered_recordset,
             num_filtered_records,
             sizeof(char **),
             clause,
             qsort_comp
           );
  #elif defined(__linux)
    qsort_r(
             filtered_recordset,
             num_filtered_records,
             sizeof(char **),
             qsort_comp,
             clause
           );
  #endif


  if(verbose_vv){

    /* initialize the output stream, if write to file is requested */
    struct io_writer_s* writer_ctxt = NULL;
    if(file) {

      /* get a file descriptor */
      char* filename = (char*)0L;
      if (asprintf(&filename, "%s/grouper-branch-%d-sorted-records.ftz",
               dirpath, branch_id) < 0)
        errExit("asprintf(...): failed");
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      writer_ctxt = io->io_write_init(read_ctxt, out_fd, num_filtered_records);
      exitOn(writer_ctxt == NULL);
    }

    /* free'd just before calling merger(...) ?*/
    gresult->sorted_recordset = (char**) calloc(num_filtered_records,
                                                sizeof(char*));
    if (gresult->sorted_recordset == NULL)
      errExit("calloc");

    /* process each sorted record */
    for (int i = 0; i < num_filtered_records; i++) {

      gresult->sorted_recordset[i] = filtered_recordset[i];

      /* write the record to the output stream */
      if(file) {
        exitOn((io->io_write_record(writer_ctxt,
                                    gresult->sorted_recordset[i])) < 0);
      }
    }

    /* close the output stream */
    if(file) {
      exitOn(io->io_write_close(writer_ctxt) < 0);
    }
  }

  // mark the end of sorted records
  filtered_recordset[num_filtered_records] = NULL;

  return filtered_recordset;
}


struct grouper_result*
grouper(
        size_t num_filter_clauses,
        struct filter_clause** const filter_clauseset,

        size_t num_grouper_clauses,
        struct grouper_clause** const grouper_clauseset,

        size_t num_aggr_clause_terms,
        struct aggr_term** const aggr_clause_termset,

        const struct filter_result* const fresult,
        int rec_size,

        struct io_handler_s* io,
        struct io_reader_s*  read_ctxt,
        int branch_id
       ) {

  /* free'd just after returning from ungrouper(...) */
  struct grouper_result* gresult = calloc(1, sizeof(struct grouper_result));
  if (gresult == NULL)
    errExit("calloc");

  /* free'd just after returning from ungrouper(...) */
  struct group** groupset = (struct group **)calloc(1, sizeof(struct group *));
  if (groupset == NULL)
    errExit("calloc");
  else
    gresult->groupset = groupset;

  /* go ahead if there is something to group */
  if (fresult->num_filtered_records > 0) {

    /* assign a aggr func for each aggr clause term */
    if (groupaggregations_enabled) {
      for (int j = 0; j < num_aggr_clause_terms; j++){

        struct aggr_term* term = aggr_clause_termset[j];
        assign_aggr_func(term);
      }
    }

    /* assign aggr func for each filter term */
    for (int i = 0; i < num_filter_clauses; i++) {

      struct filter_clause* fclause = filter_clauseset[i];

      for (int j = 0; j < fclause->num_terms; j++) {

        struct filter_term* term = fclause->termset[j];
        term->aggr_function = get_aggr_fptr(false, term->op->field_type);
        if(term->aggr_function == NULL)
          errExit("get_aggr_fptr(...) returned NULL");
      }
    }

    /* assign aggr func for each grouper term */
    for (int i = 0; i < num_grouper_clauses; i++) {

      struct grouper_clause* gclause = grouper_clauseset[i];

      for (int j = 0; j < gclause->num_terms; j++) {

        struct grouper_term* term = gclause->termset[j];
        term->aggr_function = get_aggr_fptr(true, term->op->field1_type);
        if(term->aggr_function == NULL)
          errExit("get_aggr_fptr(...) returned NULL");
      }
    }

    /* open a output stream, if a file write is requested */
    io_writer_t* writer_ctxt = NULL;
    int n_groups = -1;
    if (verbose_v && file) {

      /* get a file descriptor */
      char* filename = (char*)0L;
      if (asprintf(&filename, "%s/grouper-branch-%d-groups.ftz",
               dirpath, branch_id) < 0) errExit("asprintf(...) returned -1");
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      writer_ctxt = io->io_write_init(read_ctxt, out_fd, gresult->num_groups);
      exitOn(writer_ctxt == NULL);
      n_groups = gresult->num_groups;
    }

    /* club all filtered records into one group,
     * if no group modules are defined */
    if (num_grouper_clauses == 0) {

      /* ----------------------------------------------------------------- */
      /*            create a cooked netflow v5 group record                */
      /* ----------------------------------------------------------------- */

      /* free'd just after returning from merger(...) */
      struct aggr_result* aresult = calloc(1, sizeof(struct aggr_result));
      if (aresult == NULL)
        errExit("calloc");

      /* free'd just after returning from merger(...) */
      struct aggr_record* group_aggr_record =
      calloc(1, sizeof(struct aggr_record));
      if (group_aggr_record == NULL)
        errExit("calloc");

      /* free'd just after returning from merger(...) */
      struct fttime* start = calloc(1, sizeof(struct fttime));
      if (start == NULL)
        errExit("calloc");

      /* free'd just after returning from merger(...) */
      struct fttime* end = calloc(1, sizeof(struct fttime));
      if (end == NULL)
        errExit("calloc");
      group_aggr_record->start = start;
      group_aggr_record->end = end;

      /* free'd just after returning from merger(...) */
      char* aggr_record = (char *)calloc(1, rec_size);
      if (aggr_record == NULL)
        errExit("calloc");
      group_aggr_record->aggr_record = aggr_record;
      aresult->aggr_record = group_aggr_record;

      /* ----------------------------------------------------------------- */

      /* groupset with space for 1 group */
      gresult->num_groups = 1;

      /* free'd just after returning from ungrouper(...) */
      struct group* group = (struct group *)calloc(1, sizeof(struct group));
      if (group == NULL)
        errExit("calloc");

      groupset[gresult->num_groups - 1] = group;
      group->num_members = fresult->num_filtered_records;

      /* free'd after returning from ungrouper(...) */
      group->members = (char **)calloc(group->num_members, sizeof(char *));
      if (group->members == NULL)
        errExit("calloc");

      /* set minimum START and maximum END timestamp */

      char* first_item = fresult->filtered_recordset[0];

      uint32_t sysUpTime =
      *(u_int32_t*)(first_item+ io->io_read_get_field_offset(read_ctxt, "sysUpTime"));
      uint32_t unix_secs =
      *(u_int32_t*)(first_item+ io->io_read_get_field_offset(read_ctxt, "unix_secs"));
      uint32_t unix_nsecs =
      *(u_int32_t*)(first_item+ io->io_read_get_field_offset(read_ctxt, "unix_nsecs"));
      uint32_t First =
      *(u_int32_t*)(first_item+ io->io_read_get_field_offset(read_ctxt, "flowStartSysUpTime"));
      uint32_t Last =
      *(u_int32_t*)(first_item+ io->io_read_get_field_offset(read_ctxt, "flowEndSysUpTime"));

      struct fttime min = ftltime(sysUpTime,unix_secs,unix_nsecs,First);
      struct fttime max = ftltime(sysUpTime,unix_secs,unix_nsecs,Last);

      group_aggr_record->start->secs = min.secs;
      group_aggr_record->start->msecs = min.msecs;
      group_aggr_record->end->secs = max.secs;
      group_aggr_record->end->msecs = max.msecs;

      for (int i = 0; i < fresult->num_filtered_records; i++) {

        char* current_item = fresult->filtered_recordset[i];

        uint32_t sysUpTime =
        *(u_int32_t*)(current_item + io->io_read_get_field_offset(read_ctxt, "sysUpTime"));
        uint32_t unix_secs =
        *(u_int32_t*)(current_item + io->io_read_get_field_offset(read_ctxt, "unix_secs"));
        uint32_t unix_nsecs =
        *(u_int32_t*)(current_item + io->io_read_get_field_offset(read_ctxt, "unix_nsecs"));
        uint32_t First =
        *(u_int32_t*)(current_item + io->io_read_get_field_offset(read_ctxt, "flowStartSysUpTime"));
        uint32_t Last =
        *(u_int32_t*)(current_item + io->io_read_get_field_offset(read_ctxt, "flowEndSysUpTime"));

        struct fttime start =
        ftltime (sysUpTime, unix_secs, unix_nsecs, First);
        struct fttime end =
        ftltime (sysUpTime, unix_secs, unix_nsecs, Last);

        /* update the START timestamp if we have a new min */
        if (
            (start.secs < min.secs) ||
            ((start.secs == min.secs) && (start.msecs < min.msecs))
            ) {
          min.secs = start.secs; min.msecs = start.msecs;
          group_aggr_record->start->secs = min.secs;
          group_aggr_record->start->msecs = min.msecs;
        }

        /* update the END timestamp if we have a new max */
        if (
            (end.secs > max.secs) ||
            ((end.secs == max.secs) && (end.msecs > max.msecs))
            ) {
          max.secs = end.secs; max.msecs = end.msecs;
          group_aggr_record->end->secs = max.secs;
          group_aggr_record->end->msecs = max.msecs;
        }

        group->members[i] = current_item;
      }

      group->aggr_result = aresult;
      if (group->aggr_result == NULL)
        errExit("grouper_aggregations(...) returned NULL");

      /* save common fields (coming from the filter rule) in aggregation
       * record currently assumes that the filter rule was `eq`, such that
       * each record member has the same value for that field in the group,
       * still need to investigate how it might affect filter operations */
      for (int i = 0; i < num_filter_clauses; i++) {

        struct filter_clause* fclause = filter_clauseset[i];

        for (int j = 0; j < fclause->num_terms; j++) {

          struct filter_term* term = fclause->termset[j];
          struct aggr* aggr = term->aggr_function (
                                                    group->members,
                                                    aggr_record,
                                                    group->num_members,
                                                    term->field_offset,
                                                    TRUE
                                                  );
          if (aggr == NULL)
            errExit("aggr_function(...) returned NULL");
          else {
            free(aggr->values); aggr->values = NULL;
            free(aggr); aggr = NULL;
          }
        }
      }

      /* ----------------------------------------------------------------- */





      /* ----------------------------------------------------------------- */
      /*                    create group aggregations                      */
      /* ----------------------------------------------------------------- */

      if (groupaggregations_enabled) {

        group->aggr_result = grouper_aggregations(
                                                    num_filter_clauses,
                                                    filter_clauseset,

                                                    num_grouper_clauses,
                                                    grouper_clauseset,

                                                    num_aggr_clause_terms,
                                                    aggr_clause_termset,

                                                    group,
                                                    aresult
                                                  );
        if (group->aggr_result == NULL)
          errExit("grouper_aggregations(...) returned NULL");
      }

      /* ----------------------------------------------------------------- */



      /* ----------------------------------------------------------------- */
      /*                    write the record to file                       */
      /* ----------------------------------------------------------------- */

      if (verbose_v && file) {

        /* write the record to the output stream */
        exitOn((n_groups =
                io->io_write_record(writer_ctxt,
                        group->aggr_result->aggr_record->aggr_record)) < 0);
      }

      /* open a output stream for group members, if a file write is requested */
      struct io_writer_s* members_writer_ctxt = NULL;
      if (verbose_vv && file) {

        /* get a file descriptor */
        char* filename = (char*)0L;
        if (asprintf(&filename, "%s/grouper-branch-%d-group-%d-records.ftz",
                 dirpath, branch_id, 0) < 0)
          errExit("asprintf(...) returned -1");
        int out_fd = get_fd(filename);
        if(out_fd == -1) errExit("get_fd(...) returned -1");
        else free(filename);

        /* get the output stream */
        members_writer_ctxt = io->io_write_init(read_ctxt,
                                                out_fd,
                                                group->num_members);
        exitOn(members_writer_ctxt == NULL);
      }

      /* close the output stream */
      if (verbose_vv && file) {
        io->io_write_close(members_writer_ctxt);
      }

      /* ----------------------------------------------------------------- */
    }
    else {

      /* assign grouper func for each term */
      for (int k = 0; k < num_grouper_clauses; k++) {

        struct grouper_clause* gclause = grouper_clauseset[k];

        for (int j = 0; j < gclause->num_terms; j++) {

          /* assign a uintX_t specific function depending on grule->op */
          struct grouper_term* term = gclause->termset[j];
          assign_grouper_func(term);
        }
      }

      /* process each grouper clause */
      for (int l = 0; l < num_grouper_clauses; l++) {

        struct grouper_clause* gclause = grouper_clauseset[l];

        /* free'd at the end of this clause loop */
        struct grouper_type** gtypeset =
        calloc(gclause->num_terms, sizeof(struct grouper_type*));
        if (gtypeset == NULL)
          errExit("calloc");

        /* get the grouper types for each term of the clause */
        for (int i = 0; i < gclause->num_terms; i++) {

          struct grouper_term* term = gclause->termset[i];
          struct grouper_type* gtype = get_gtype(term->op->field2_type);
          if (gtype == NULL)
            errExit("get_type(...) returned NULL");

          gtypeset[i] = gtype;
        }
        gclause->gtypeset = gtypeset;

        char** sorted_recordset_reference =
        get_grouper_intermediates(
                                   fresult->num_filtered_records,
                                   fresult->filtered_recordset,

                                   gclause,
                                   gresult,

                                   io,
                                   read_ctxt,

                                   branch_id
                                 );

        if (sorted_recordset_reference == NULL)
          errExit("get_grouper_intermediates(...) returned NULL");

        /* an item from the sorted filtered recordset */
        char** current_item = sorted_recordset_reference;

        char** last_item =
        sorted_recordset_reference + (fresult->num_filtered_records - 1);

        /* process each filtered record for grouping */
        for (int k = 0; *current_item != NULL; current_item++) {

          /* -------------------------------------------------------------- */
          /*          create a cooked netflow v5 group record               */
          /* -------------------------------------------------------------- */

          /* free'd just after returning from merger(...) */
          struct aggr_result* aresult = calloc(1, sizeof(struct aggr_result));
          if (aresult == NULL)
            errExit("calloc");

          /* free'd just after returning from merger(...) */
          struct aggr_record* group_aggr_record =
          calloc(1, sizeof(struct aggr_record));
          if (group_aggr_record == NULL)
            errExit("calloc");

          /* free'd just after returning from merger(...) */
          struct fttime* start = calloc(1, sizeof(struct fttime));
          if (start == NULL)
            errExit("calloc");

          /* free'd just after returning from merger(...) */
          struct fttime* end = calloc(1, sizeof(struct fttime));
          if (end == NULL)
            errExit("calloc");
          group_aggr_record->start = start;
          group_aggr_record->end = end;

          /* free'd just after returning from merger(...) */
          char* aggr_record = (char *)calloc(1, rec_size);
          if (aggr_record == NULL)
            errExit("calloc");
          group_aggr_record->aggr_record = aggr_record;
          aresult->aggr_record = group_aggr_record;

          /* -------------------------------------------------------------- */

          /* create a new group out of this item */
          gresult->num_groups += 1;

          /* free'd just after returning from ungrouper(...) */
          groupset = (struct group **)
          realloc(groupset, (gresult->num_groups)*sizeof(struct group*));
          if (groupset == NULL)
            errExit("realloc");
          else
            gresult->groupset = groupset;

          /* free'd just after returning from ungrouper(...) */
          struct group* group = (struct group *)calloc(1, sizeof(struct group));
          if (group == NULL)
            errExit("calloc");

          groupset[gresult->num_groups-1] = group;
          group->num_members = 1;

          /* free'd after returning from ungrouper(...) */
          group->members = (char **)calloc(1, sizeof(char *));
          if (group->members == NULL)
            errExit("calloc");
          group->members[0] = *current_item;

          /* set minimum START and maximum END timestamp */
          uint32_t sysUpTime =
          *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "sysUpTime"));
          uint32_t unix_secs =
          *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "unix_secs"));
          uint32_t unix_nsecs =
          *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "unix_nsecs"));
          uint32_t First =
          *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "flowStartSysUpTime"));
          uint32_t Last =
          *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "flowEndSysUpTime"));

          struct fttime min = ftltime(sysUpTime,unix_secs,unix_nsecs,First);
          struct fttime max = ftltime(sysUpTime,unix_secs,unix_nsecs,Last);

          group_aggr_record->start->secs = min.secs;
          group_aggr_record->start->msecs = min.msecs;
          group_aggr_record->end->secs = max.secs;
          group_aggr_record->end->msecs = max.msecs;

          /* initialize an output stream, if file write is requested */
          if(verbose_vv && file) {

            /* get a file descriptor */
            char* filename = (char*)0L;
            if (asprintf(&filename, "%s/grouper-branch-%d-group-%d-records.ftz",
                     dirpath, branch_id, gresult->num_groups - 1) < 0)
              errExit("asprintf(...): failed");
            int out_fd = get_fd(filename);
            if(out_fd == -1) errExit("get_fd(...) returned -1");
            else free(filename);

            /* get the output stream */
            writer_ctxt = io->io_write_init(read_ctxt, out_fd, group->num_members);
          }

          while (current_item != last_item) {

            current_item++;

            // check all terms for this grouper clause for those two records
            for (k = 0; k < gclause->num_terms; k++) {

              struct grouper_term* term = gclause->termset[k];

              if (
                  !term->func(
                              group,
                              term->field_offset1,
                              *current_item,
                              term->field_offset2,
                              term->delta
                              )
                  )
                break;
            }

            // break out, if one of the terms didnt match
            if (k < gclause->num_terms) {

              // go back to the last item that matched
              current_item--;
              break;
            }

            // all the terms matched!
            else {

              // add this member to the group
              group->num_members += 1;
              group->members = (char **) realloc(group->members,
                                            sizeof(char*)*group->num_members);
              if (group->members == NULL)
                errExit("realloc");
              group->members[group->num_members-1] = *current_item;

              uint32_t sysUpTime =
              *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "sysUpTime"));
              uint32_t unix_secs =
              *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "unix_secs"));
              uint32_t unix_nsecs =
              *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "unix_nsecs"));
              uint32_t First =
              *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "flowStartSysUpTime"));
              uint32_t Last =
              *(u_int32_t*)(*current_item + io->io_read_get_field_offset(read_ctxt, "flowEndSysUpTime"));

              struct fttime start =
              ftltime (sysUpTime, unix_secs, unix_nsecs, First);
              struct fttime end =
              ftltime (sysUpTime, unix_secs, unix_nsecs, Last);

              /* update the START timestamp if we have a new min */
              if (
                  (start.secs < min.secs) ||
                  ((start.secs == min.secs) && (start.msecs < min.msecs))
                 ) {
                min.secs = start.secs; min.msecs = start.msecs;
                group_aggr_record->start->secs = min.secs;
                group_aggr_record->start->msecs = min.msecs;
              }

              /* update the END timestamp if we have a new max */
              if (
                  (end.secs > max.secs) ||
                  ((end.secs == max.secs) && (end.msecs > max.msecs))
                 ) {
                max.secs = end.secs; max.msecs = end.msecs;
                group_aggr_record->end->secs = max.secs;
                group_aggr_record->end->msecs = max.msecs;
              }

              // write this member to the file (if requested)
              if (verbose_vv && file) {

                /* write the record to the output stream */
                exitOn(io->io_write_record(writer_ctxt, *current_item) < 0);
              }
            }
          }

          /* close the output stream */
          if (verbose_vv && file) {
            exitOn(io->io_write_close(writer_ctxt) < 0);
          }

        /* ----------------------------------------------------------------- */
        /*            create a cooked netflow v5 group record                */
        /* ----------------------------------------------------------------- */

          group->aggr_result = aresult;
          if (group->aggr_result == NULL)
            errExit("grouper_aggregations(...) returned NULL");

          /* save common fields (coming from the filter rule) in aggregation
           * record currently assumes that the filter rule was `eq`, such that
           * each record member has the same value for that field in the group,
           * still need to investigate how it might affect filter operations */
          for (int i = 0; i < num_filter_clauses; i++) {

            struct filter_clause* fclause = filter_clauseset[i];

            for (int j = 0; j < fclause->num_terms; j++) {

              struct filter_term* term = fclause->termset[j];
              struct aggr* aggr = term->aggr_function (
                                                        group->members,
                                                        aggr_record,
                                                        group->num_members,
                                                        term->field_offset,
                                                        TRUE
                                                      );
              if (aggr == NULL)
                errExit("aggr_function(...) returned NULL");
              else {
                free(aggr->values); aggr->values = NULL;
                free(aggr); aggr = NULL;
              }
            }
          }

          /* save common fields (coming from the grouper rule) in aggregation
           * record; currently assumes that the grouper rule was `eq`, such
           * that each record member has the same value for that field in the
           * group, still need to investigate how it might affect other
           * grouper operations */
          for (int i = 0; i < num_grouper_clauses; i++) {

            struct grouper_clause* gclause = grouper_clauseset[i];

            for (int j = 0; j < gclause->num_terms; j++) {

              struct grouper_term* term = gclause->termset[j];

              size_t goffset_1 = term->field_offset1;
              size_t goffset_2 = term->field_offset2;

              if(goffset_1 != goffset_2) {

                struct aggr* aggr = term->aggr_function (
                                                          group->members,
                                                          aggr_record,
                                                          group->num_members,
                                                          goffset_1,
                                                          TRUE
                                                        );
                if (aggr == NULL)
                  errExit("aggr_function(...) returned NULL");
                else {
                  free(aggr->values); aggr->values = NULL;
                  free(aggr); aggr =  NULL;
                }
              }

              struct aggr* aggr = term->aggr_function (
                                                        group->members,
                                                        aggr_record,
                                                        group->num_members,
                                                        goffset_2,
                                                        TRUE
                                                      );
              if (aggr == NULL)
                errExit("aggr_function(...) returned NULL");
              else {
                free(aggr->values); aggr->values = NULL;
                free(aggr); aggr = NULL;
              }
            }
          }

        /* ----------------------------------------------------------------- */


        /* ----------------------------------------------------------------- */
        /*                    create group aggregations                      */
        /* ----------------------------------------------------------------- */

          if (groupaggregations_enabled) {

            group->aggr_result = grouper_aggregations(
                                                       num_filter_clauses,
                                                       filter_clauseset,

                                                       num_grouper_clauses,
                                                       grouper_clauseset,

                                                       num_aggr_clause_terms,
                                                       aggr_clause_termset,

                                                       group,
                                                       aresult
                                                     );
            if (group->aggr_result == NULL)
              errExit("grouper_aggregations(...) returned NULL");
          }

        /* ----------------------------------------------------------------- */


        /* ----------------------------------------------------------------- */
        /*                    write to the output stream                     */
        /* ----------------------------------------------------------------- */

          if (verbose_v && file) {
            exitOn((n_groups = io->io_write_record(writer_ctxt,
                              group->aggr_result->aggr_record->aggr_record)
                             ) < 0);
          }

        /* ----------------------------------------------------------------- */


        }

        // unlink the sorted records from the flow data
        sorted_recordset_reference = NULL;

        /* free the grouper types for each term of this clause  */
        for (int i = 0; i < gclause->num_terms; i++) {
          struct grouper_type* gtype = gclause->gtypeset[i];
          free(gtype); gtype = NULL; gclause->gtypeset[i] = NULL;
        }
        free(gclause->gtypeset); gclause->gtypeset = NULL;
      }
    }

    /* close the output stream */
    if (verbose_v && file) {
      io->io_write_close(writer_ctxt);
    }
  }

  return gresult;
}


#if defined(__APPLE__) || defined(__FreeBSD__)
int qsort_comp(void *thunk, const void *e1, const void *e2) {
#elif defined(__linux)
int qsort_comp(const void *e1, const void *e2, void *thunk) {
#endif
  int result = 0;
  struct grouper_clause* clause = (struct grouper_clause*)thunk;
  struct grouper_term** termset = clause->termset;
  struct grouper_type** gtypeset = clause->gtypeset;

  for (int i = 0; i < clause->num_terms; i++) {

    struct grouper_type* gtype = gtypeset[i];
    struct grouper_term* term = termset[i];

#if defined(__APPLE__) || defined(__FreeBSD__)
    result = gtype->qsort_comp((void*)term->field_offset2, e1, e2);
#elif defined(__linux)
    result = gtype->qsort_comp(e1, e2, (void*)term->field_offset2);
#endif

    if (result != 0) break;
  }

  return result;
}
