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

#include "io-ft.h"

#include "io.h"

#include <assert.h>

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

io_reader_t*
io_ft_read_init(io_ctxt_t* io_ctxt, int read_fd) {
  UNUSED(io_ctxt);

  struct ft_data* data = ft_init(read_fd);
  io_reader_t* io_reader = (io_reader_t*) data;

  assert(&io_reader->d.ft == data); /* XXX This may be guaranteed by the
                                       standard */

  return io_reader;
}

char*
io_ft_read_record(io_reader_t* io_reader) {
  return ftio_read(&io_reader->d.ft.io);
}

size_t
io_ft_read_get_field_offset(io_reader_t* io_reader, const char* field) {
  return io_ft_get_offset(field, &io_reader->d.ft.offsets);
}

int
io_ft_read_close(io_reader_t* io_reader) {
  ft_close(&io_reader->d.ft);

  return 0;
}

void
io_ft_print_header(io_reader_t* io_reader) {
  ftio_header_print(&io_reader->d.ft.io, stdout, '#');
}

void
io_ft_print_record(io_reader_t* io_reader, char* record) {
  flow_print_record(&io_reader->d.ft, record);
}

void
io_ft_print_aggr_record(io_reader_t* io_reader,
                        struct aggr_record* aggr_record) {
  flow_print_group_record(&io_reader->d.ft, aggr_record);
}

io_writer_t*
io_ft_write_init(io_reader_t* io_reader,
                 int write_fd,
                 int num_records) {
  // XXX union padding guarantees
  return (io_writer_t*) get_ftio(&io_reader->d.ft, write_fd, num_records);
}

int
io_ft_write_record(io_writer_t* io_writer, char* record) {
  return ftio_write(&io_writer->d.ft, record);
}

int io_ft_write_close(io_writer_t* io_writer) {
  return ftio_close(&io_writer->d.ft);
}


/* Legacy methods */
/*--------------------------------------------------------------------------*/
/* Legacy methods                                                           */
/*--------------------------------------------------------------------------*/
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

  /* initialize the output stream if file write is requested */
  if (verbose_v && file) {

    for (int i = 0; i < fquery->num_branches; i++) {

      struct branch* branch = fquery->branchset[i];

      /* get a file descriptor */
      char* filename = (char*)0L;
      if (asprintf(&filename, "%s/filter-branch-%d-filtered-records.ftz",
               dirpath, branch->branch_id) < 0)
        errExit("asprintf(...): failed");
      int out_fd = get_fd(filename);
      if(out_fd == -1) errExit("get_fd(...) returned -1");
      else free(filename);

      /* get the output stream */
      branch->ftio_out = get_ftio(
                                  data,
                                  out_fd,
                                  branch->filter_result->num_filtered_records
                                 );
      /* write the header to the output stream */
      if (ftio_write_header(branch->ftio_out) < 0)
        fterr_errx(1, "ftio_write_header(): failed");
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

        /* write to the output stream, if requested */
        if (verbose_v && file) {
          if ((ftio_write(branch->ftio_out, record)) < 0)
            fterr_errx(1, "ftio_write(): failed");
        }
      }
    }
  }

  /* close the output stream */
  if (verbose_v && file) {
    for (int i = 0; i < fquery->num_branches; i++) {

      struct branch* branch = fquery->branchset[i];

      if ((ftio_close(branch->ftio_out)) < 0)
        fterr_errx(1, "ftio_close(): failed");
      free(branch->ftio_out);
    }
  }

  /* set the last item in filtered_recordset to NULL */
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


size_t
io_ft_get_offset(const char * const name,
                 const struct fts3rec_offsets* const offsets) {

  /* TODO fix temporary hack */
  #define CASEOFF_IPFIX(ipfix_ie, memb)       \
  if (strcmp(name, #ipfix_ie) == 0)           \
    return offsets->memb

  #define CASEOFF(memb)                       \
  if (strcmp(name, #memb) == 0)               \
    return offsets->memb

	CASEOFF(unix_secs);
	CASEOFF(unix_nsecs);
	CASEOFF(sysUpTime);
	CASEOFF(exaddr);
	CASEOFF_IPFIX(sourceIPv4Address, srcaddr);
	CASEOFF_IPFIX(destinationIPv4Address, dstaddr);
	CASEOFF_IPFIX(ipNextHopIPv4Address, nexthop);
	CASEOFF(input);
	CASEOFF(output);
	CASEOFF_IPFIX(deltaFlowCount, dFlows);
	CASEOFF_IPFIX(packetDeltaCount, dPkts);
	CASEOFF_IPFIX(octetDeltaCount, dOctets);
	CASEOFF_IPFIX(flowStartSysUpTime, First);
	CASEOFF_IPFIX(flowEndSysUpTime, Last);
	CASEOFF_IPFIX(tcpSourcePort, srcport);
	CASEOFF_IPFIX(udpSourcePort, srcport);
	CASEOFF_IPFIX(tcpDestinationPort, dstport);
	CASEOFF_IPFIX(udpDestinationPort, dstport);
	CASEOFF_IPFIX(protocolIdentifier, prot);
	CASEOFF_IPFIX(ipClassOfService, tos);
	CASEOFF_IPFIX(tcpControlBits, tcp_flags);
	CASEOFF(pad);
	CASEOFF(engine_type);
	CASEOFF(engine_id);
	CASEOFF(src_mask);
	CASEOFF(dst_mask);
	CASEOFF_IPFIX(bgpSourceAsNumber, src_as);
	CASEOFF_IPFIX(bgpDestinationAsNumber, dst_as);
	CASEOFF(in_encaps);
	CASEOFF(out_encaps);
	CASEOFF_IPFIX(ipNextHopIPv4Address, peer_nexthop);
	CASEOFF(router_sc);
	CASEOFF(src_tag);
	CASEOFF(dst_tag);
	CASEOFF(extra_pkts);
	CASEOFF(marked_tos);

  #undef CASEOFF_IPFIX
  #undef CASEOFF

  return -1;
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

void
flow_print_group_record(struct ft_data *data, struct aggr_record* aggr_record){

  char* record = aggr_record->aggr_record;

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
  ftt = *aggr_record->start;
  struct tm *tm;
  time_t t_first = ftt.secs;
  tm = localtime(&t_first);

  printf("%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu ",
         (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
         (int)tm->tm_min, (int)tm->tm_sec, (u_long)ftt.msecs);

  ftt = *aggr_record->end;
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


struct ftio*
get_ftio(
         struct ft_data* const dataformat,
         int out_fd,
         uint32_t total_flows
        ) {

  /* initialize output ftio structure */
  struct ftio* ftio_out = calloc(1, sizeof(struct ftio));
  if (ftio_out == NULL)
    errExit("calloc");

  struct ftset ftset; ftset_init(&ftset, 0);
  /* defaults to 5 */
  ftset.z_level = zlevel;
  if (ftio_init(ftio_out, out_fd, FT_IO_FLAG_WRITE |
    ((ftset.z_level) ? FT_IO_FLAG_ZINIT : 0) ) < 0)
    fterr_errx(1, "ftio_init(): failed");

  /* set compression level */
  ftio_set_z_level(ftio_out, ftset.z_level);

  /* set endianess */
  ftset.byte_order = FT_HEADER_LITTLE_ENDIAN;
  ftio_set_byte_order(ftio_out, ftset.byte_order);

  ftio_set_streaming(ftio_out, 1);
  ftio_set_debug(ftio_out, debug);
  ftio_set_preloaded(ftio_out, 1);

  /* set total, corrupt and lost flows */
  uint32_t corrupt_flows = 0;uint32_t lost_flows = 0;
  ftio_set_flows_count(ftio_out, total_flows);
  ftio_set_corrupt(ftio_out, corrupt_flows);
  ftio_set_lost(ftio_out, lost_flows);

  /* set the hostname and comments */
  struct ftio ftio_in = dataformat->io;
  if (ftio_get_hostname(&ftio_in))
    ftio_set_cap_hostname(ftio_out, ftio_get_hostname(&ftio_in));
  if (ftio_get_comment(&ftio_in))
    ftio_set_comment(ftio_out, ftio_get_comment(&ftio_in));
  if (ftio_in.fth.fields & FT_FIELD_SEQ_RESET)
    ftio_set_reset(ftio_out, ftio_in.fth.seq_reset);

  /* set the version information */
  struct ftver ftv2; bzero(&ftv2, sizeof ftv2);
  ftio_get_ver(&ftio_in, &ftv2);
  if (ftio_set_ver(ftio_out, &ftv2) < 0)
    fterr_errx(1, "ftio_set_ver(): failed");

  return ftio_out;
}
