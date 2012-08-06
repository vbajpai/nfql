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

#include "ftreader.h"

struct ft_data *
ft_init(int fd) {

  struct ft_data* data = (struct ft_data *)calloc(1, sizeof(struct ft_data));
  if(data == NULL)
    errExit("calloc");

  data->fd = fd;

  int ret = ftio_init(&data->io, data->fd, FT_IO_FLAG_READ);
  if (ret < 0)
    errExit("ftio_init");

  if (ftio_check_xfield(&data->io,
                        FT_XFIELD_DPKTS      |
                        FT_XFIELD_DOCTETS    |
                        FT_XFIELD_FIRST      |
                        FT_XFIELD_LAST       |
                        FT_XFIELD_INPUT      |
                        FT_XFIELD_OUTPUT     |
                        FT_XFIELD_SRCADDR    |
                        FT_XFIELD_DSTADDR    |
                        FT_XFIELD_SRCPORT    |
                        FT_XFIELD_DSTPORT    |
                        FT_XFIELD_UNIX_SECS  |
                        FT_XFIELD_UNIX_NSECS |
                        FT_XFIELD_SYSUPTIME  |
                        FT_XFIELD_TCP_FLAGS  |
                        FT_XFIELD_PROT)) {
    fterr_warnx("Flow record missing required field for format.");
    exit(EXIT_FAILURE);
  }else {
    /* calculate FT_XFIELD */
    data->xfield = ftio_xfield(&data->io);
  }

  ftio_get_ver(&data->io, &data->version);
  fts3rec_compute_offsets(&data->offsets, &data->version);
  data->rec_size = ftio_rec_size(&data->io);
  return data;
}

struct ft_data *
ft_read(
         struct ft_data* data,
         struct flowquery* fquery
       ) {


  /* assign filter func for all the branches */
  if (filter_enabled) {
    for (int i = 0; i < fquery->num_branches; i++) {

      struct branch* branch = fquery->branchset[i];

      /* free'd before exiting from main(...) */
      branch->filter_result = calloc(1, sizeof(struct filter_result));
      if (branch->filter_result == NULL)
        errExit("calloc");

      /* free'd before exiting from main(...) */
      branch->filter_result->filtered_recordset = (char **)
      calloc(branch->filter_result->num_filtered_records, sizeof(char *));
      if (branch->filter_result->filtered_recordset == NULL)
        errExit("calloc");

      /* assign a filter func for each filter rule */
      for (int k = 0; k < branch->num_filter_clauses; k++) {

        struct filter_clause* const fclause = branch->filter_clauseset[k];

        for (int j = 0; j < fclause->num_terms; j++) {

          struct filter_term* const term = fclause->termset[j];

          /* get a uintX_t specific function depending on frule->op */
          assign_filter_func(term);
          fclause->termset[j] = term;
        }
      }
    }
  }

  /* display the flow-header when --debug/--verbose=3 is SET */
  if(verbose_vvv && !file){

    /* print flow header */
    ftio_header_print(&data->io, stdout, '#');

    puts(FLOWHEADER);
  }

  char* record = NULL;
  /* process each flow record */
  while ((record = ftio_read(&data->io)) != NULL) {

    /* display each record when --debug/--verbose=3 is SET */
    if(verbose_vvv && !file)
      flow_print_record(data, record);

    /* process each branch */
    char* target = NULL; bool first_time = true;
    for (int i = 0, j; i < fquery->num_branches; i++) {

      struct branch* branch = fquery->branchset[i]; bool satisfied = false;

      /* process each filter clause (clauses are OR'd) */
      for (int k = 0; k < branch->num_filter_clauses; k++) {

        struct filter_clause* const fclause = branch->filter_clauseset[k];

        /* process each filter term (terms within a clause are AND'd) */
        for (j = 0; j < fclause->num_terms; j++) {

          struct filter_term* const term = fclause->termset[j];

          /* run the comparator function of the filter rule on the record */
          if (!term->func(
                          record,
                          term->field_offset,
                          term->value,
                          term->delta
                         ))
            break;
        }

        /* if any rule is not satisfied, move to the next module */
        if (j < fclause->num_terms)
          continue;
        /* else this module is TRUE; so everything is TRUE; break out */
        else {
          satisfied = true; break;
        }
      }

      /* if rules are satisfied then save this record */
      if (satisfied) {

        /* save this record in the trace data only once for all the
         * branches to refer to */
        if(first_time) {

          /* allocate memory for the record */
          target = (char*) calloc(1, data->rec_size);
          if (target == NULL)
            errExit("calloc(...)");

          /* copy the record */
          memcpy(target, record, data->rec_size);

          /* save the record in the trace data */
          data->num_records += 1;
          data->recordset = (char**) realloc(
                                              data->recordset,
                                              data->num_records * sizeof(char*)
                                            );
          if(data->recordset)
          data->recordset[data->num_records - 1] = target;
          first_time = false;
        }

        /* increase the filtered recordset size */
        branch->filter_result->num_filtered_records += 1;
        branch->filter_result->filtered_recordset = (char **)
                         realloc(branch->filter_result->filtered_recordset,
                                (branch->filter_result->num_filtered_records)
                                 *sizeof(char *));
        if (branch->filter_result->filtered_recordset == NULL)
          errExit("realloc");

        /* also save the pointer in the filtered recordset */
        branch->filter_result->
        filtered_recordset[branch->filter_result->
                           num_filtered_records - 1] = target;
      }
    }
  }
  
  if (filter_enabled) {
    
    for (int i = 0; i < fquery->num_branches; i++) {
      
      struct branch* branch = fquery->branchset[i];
  
      /* add one more member and assign it to NULL */
      branch->filter_result->filtered_recordset = (char **)
      realloc( branch->filter_result->filtered_recordset,
               (branch->filter_result->num_filtered_records + 1) * sizeof(char*)
             );
      if (branch->filter_result->filtered_recordset == NULL)
        errExit("realloc");
      
      branch->filter_result->filtered_recordset
      [branch->filter_result->num_filtered_records] = NULL;
    }
  }

  if (filter_enabled) {

    for (int i = 0; i < fquery->num_branches; i++) {

      struct branch* branch = fquery->branchset[i];

      /* add one more member and assign it to NULL */
      branch->filter_result->filtered_recordset = (char **)
      realloc( branch->filter_result->filtered_recordset,
               (branch->filter_result->num_filtered_records + 1) * sizeof(char*)
             );
      if (branch->filter_result->filtered_recordset == NULL)
        errExit("realloc");

      branch->filter_result->filtered_recordset
      [branch->filter_result->num_filtered_records] = NULL;
    }
  }

  /* print the filtered records if verbose mode is set */
  if (filter_enabled) {
    if (verbose_v) {
      /* process each branch */
      for (int i = 0; i < fquery->num_branches; i++) {
        struct branch* branch = fquery->branchset[i];

#ifdef FILTER
        echo_filter(
                    branch->branch_id,
                    branch->filter_result,
                    data
                    );
#endif
      }
    }
  }
  return data;
}

void
ft_records_get_all(struct ft_data* data, int number,
                   struct fts3rec_all *record) {
  record->unix_secs = ft_records_get_unix_secs(data, number);
  record->unix_nsecs = ft_records_get_unix_nsecs(data, number);
  record->sysUpTime = ft_records_get_sysUpTime(data, number);
  record->exaddr = ft_records_get_exaddr(data, number);
  record->srcaddr = ft_records_get_srcaddr(data, number);
  record->dstaddr = ft_records_get_dstaddr(data, number);
  record->nexthop = ft_records_get_nexthop(data, number);
  record->input = ft_records_get_input(data, number);
  record->output = ft_records_get_output(data, number);
  record->dFlows = ft_records_get_dFlows(data, number);
  record->dPkts = ft_records_get_dPkts(data, number);
  record->dOctets = ft_records_get_dOctets(data, number);
  record->First = ft_records_get_First(data, number);
  record->Last = ft_records_get_Last(data, number);
  record->srcport = ft_records_get_srcport(data, number);
  record->dstport = ft_records_get_dstport(data, number);
  record->prot = ft_records_get_prot(data, number);
  record->tos = ft_records_get_tos(data, number);
  record->tcp_flags = ft_records_get_tcp_flags(data, number);
  record->engine_type = ft_records_get_engine_type(data, number);
  record->engine_id = ft_records_get_engine_id(data, number);
  record->src_mask = ft_records_get_src_mask(data, number);
  record->dst_mask = ft_records_get_dst_mask(data, number);
  record->src_as = ft_records_get_src_as(data, number);
  record->dst_as = ft_records_get_dst_as(data, number);
  record->in_encaps = ft_records_get_in_encaps(data, number);
  record->out_encaps = ft_records_get_out_encaps(data, number);
  record->peer_nexthop = ft_records_get_peer_nexthop(data, number);
  record->router_sc = ft_records_get_router_sc(data, number);
  record->src_tag = ft_records_get_src_tag(data, number);
  record->dst_tag = ft_records_get_dst_tag(data, number);
  record->extra_pkts = ft_records_get_extra_pkts(data, number);
  record->marked_tos = ft_records_get_marked_tos(data, number);
}

u_int32_t *
ft_records_get_unix_secs(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.unix_secs);
}

u_int32_t *
ft_records_get_unix_nsecs(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.unix_nsecs);
}

u_int32_t *
ft_records_get_sysUpTime(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.sysUpTime);
}

u_int32_t *
ft_records_get_exaddr(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.exaddr);
}

u_int32_t *
ft_records_get_srcaddr(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.srcaddr);
}

u_int32_t *
ft_records_get_dstaddr(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.dstaddr);
}

u_int32_t *
ft_records_get_nexthop(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.nexthop);
}

u_int16_t *
ft_records_get_input(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] + data->offsets.input);
}

u_int16_t *
ft_records_get_output(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] + data->offsets.output);
}

u_int32_t *
ft_records_get_dFlows(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] + data->offsets.dFlows);
}

u_int32_t *
ft_records_get_dPkts(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.dPkts);
}

u_int32_t *
ft_records_get_dOctets(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.dOctets);
}

u_int32_t *
ft_records_get_First(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.First);
}

u_int32_t *
ft_records_get_Last(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.Last);
}

u_int16_t *
ft_records_get_srcport(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] +  data->offsets.srcport);
}

u_int16_t *
ft_records_get_dstport(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] +  data->offsets.dstport);
}

u_int8_t *
ft_records_get_prot(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.prot);
}

u_int8_t *
ft_records_get_tos(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.tos);
}

u_int8_t *
ft_records_get_tcp_flags(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.tcp_flags);
}

u_int8_t *
ft_records_get_engine_type(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.engine_type);
}

u_int8_t *
ft_records_get_engine_id(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.engine_id);
}

u_int8_t *
ft_records_get_src_mask(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.src_mask);
}

u_int8_t *
ft_records_get_dst_mask(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.dst_mask);
}

u_int16_t *
ft_records_get_src_as(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] +  data->offsets.src_as);
}

u_int16_t *
ft_records_get_dst_as(struct ft_data* data, int number) {
  return (u_int16_t *)(data->recordset[number] +  data->offsets.dst_as);
}

u_int8_t  *
ft_records_get_in_encaps(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.in_encaps);
}

u_int8_t  *
ft_records_get_out_encaps(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.out_encaps);
}

u_int32_t *
ft_records_get_peer_nexthop(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.peer_nexthop);
}

u_int32_t *
ft_records_get_router_sc(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.router_sc);
}

u_int32_t *
ft_records_get_src_tag(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.src_tag);
}

u_int32_t *
ft_records_get_dst_tag(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.dst_tag);
}

u_int32_t *
ft_records_get_extra_pkts(struct ft_data* data, int number) {
  return (u_int32_t *)(data->recordset[number] +  data->offsets.extra_pkts);
}

u_int8_t  *
ft_records_get_marked_tos(struct ft_data* data, int number) {
  return (u_int8_t  *)(data->recordset[number] +  data->offsets.marked_tos);
}

void
ft_close(struct ft_data* data) {

  ftio_close(&data->io);
  for (int i=0; i<data->num_records; i++) {
    free(data->recordset[i]); data->recordset[i] = NULL;
  }
  free(data->recordset); data->recordset = NULL;

  if(data->fd)
    close(data->fd);
  free(data); data = NULL;
}

void
flow_print_record(struct ft_data *data, char *record){

  struct fts3rec_all cur;
  cur.unix_secs = ((u_int32_t*)(record+(data->offsets).unix_secs));
  cur.unix_nsecs = ((u_int32_t*)(record+(data->offsets).unix_nsecs));
  cur.sysUpTime = ((u_int32_t*)(record+(data->offsets).sysUpTime));
  cur.dOctets = ((u_int32_t*)(record+(data->offsets).dOctets));
  cur.dPkts = ((u_int32_t*)(record+(data->offsets).dPkts));
  cur.First = ((u_int32_t*)(record+(data->offsets).First));
  cur.Last = ((u_int32_t*)(record+(data->offsets).Last));
  cur.srcaddr = ((u_int32_t*)(record+(data->offsets).srcaddr));
  cur.dstaddr = ((u_int32_t*)(record+(data->offsets).dstaddr));
  cur.input = ((u_int16_t*)(record+(data->offsets).input));
  cur.output = ((u_int16_t*)(record+(data->offsets).output));
  cur.srcport = ((u_int16_t*)(record+(data->offsets).srcport));
  cur.dstport = ((u_int16_t*)(record+(data->offsets).dstport));
  cur.prot = ((u_int8_t*)(record+(data->offsets).prot));
  cur.tcp_flags = ((u_int8_t*)(record+(data->offsets).tcp_flags));

  struct fttime
  ftt = ftltime(*cur.sysUpTime, *cur.unix_secs, *cur.unix_nsecs, *cur.First);
  struct tm *tm;
  time_t t_first = ftt.secs;
  tm = localtime(&t_first);

  printf("%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu ",
         (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
         (int)tm->tm_min, (int)tm->tm_sec, (u_long)ftt.msecs);

  ftt = ftltime(*cur.sysUpTime, *cur.unix_secs, *cur.unix_nsecs, *cur.Last);
  time_t t_last = ftt.secs;
  tm = localtime(&t_last);

  printf("%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu ",
         (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
         (int)tm->tm_min, (int)tm->tm_sec, (u_long)ftt.msecs);

  /* other info */
  char fmt_buf1[64], fmt_buf2[64];
  fmt_ipv4(fmt_buf1, *cur.srcaddr, FMT_PAD_RIGHT);
  fmt_ipv4(fmt_buf2, *cur.dstaddr, FMT_PAD_RIGHT);

  printf("%-5u %-15.15s %-5u %-5u %-15.15s %-5u %-3u %-2d %-10lu %-10lu\n",

         (u_int)*cur.input, fmt_buf1, (u_int)*cur.srcport,
         (u_int)*cur.output, fmt_buf2, (u_int)*cur.dstport,
         (u_int)*cur.prot,
         (u_int)*cur.tcp_flags & 0x7,
         (u_long)*cur.dPkts,
         (u_long)*cur.dOctets);

  if (0 & FT_OPT_NOBUF)
    fflush(stdout);
}
