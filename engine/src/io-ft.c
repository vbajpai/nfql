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
#include "errorhandlers.h"

#include <assert.h>

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* I/O handler                                                              */
/*--------------------------------------------------------------------------*/

static struct io_reader_s*  io_ft_read_init(struct io_ctxt_s* io_ctxt,
                                            int read_fd);
static char*    io_ft_read_record(struct io_reader_s* read_ctxt);
static size_t   io_ft_read_get_field_offset(struct io_reader_s* read_ctxt,
                                            const char* field);
static size_t   io_ft_read_get_record_size(struct io_reader_s* read_ctxt);
static int  io_ft_read_close(struct io_reader_s* io_reader);

static void io_ft_print_header(struct io_reader_s* io_reader);
static void io_ft_print_debug_header(struct io_reader_s* io_reader);
static void io_ft_print_record(struct io_reader_s* io_reader,
                               char* record);
static void io_ft_print_aggr_record(struct io_reader_s* io_reader,
                                    struct aggr_record* aggr_record);

static uint64_t io_ft_record_get_StartTS(struct io_reader_s* read_ctxt,
                                         char* record);
static uint64_t io_ft_record_get_EndTS(struct io_reader_s* read_ctxt,
                                       char* record);

static struct io_writer_s*  io_ft_write_init(struct io_reader_s* read_ctxt,
                                             int write_fd,
                                             uint32_t num_records);
static int  io_ft_write_record(struct io_writer_s* io_writer,
                               char* record);
static int  io_ft_write_close(struct io_writer_s* io_writer);
static void io_ft_ctxt_destroy(io_ctxt_t* io_ctxt);

/* Legacy methods */

static struct ft_data* ft_init(int fsock);
static size_t io_ft_get_offset(const char * const name,
                               const struct fts3rec_offsets* const offsets);
static void ft_close(struct ft_data* data);
static void flow_print_record(struct ft_data *, char *);
static void flow_print_group_record(struct ft_data *data,
                                    struct aggr_record* aggr_record);
static struct ftio* get_ftio(struct ft_data* const dataformat,
                             int out_fd,
                             uint32_t total_flows);

/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

struct io_handler_s*
ft_io_handler(void) {
  struct io_handler_s* io = calloc(1, sizeof(io_handler_t));
  exitOn(io == NULL);

  io->io_read_init             = io_ft_read_init;
  io->io_read_record           = io_ft_read_record;
  io->io_read_get_field_offset = io_ft_read_get_field_offset;
  io->io_read_get_record_size  = io_ft_read_get_record_size;
  io->io_read_close            = io_ft_read_close;

  io->io_print_header          = io_ft_print_header;
  io->io_print_debug_header    = io_ft_print_debug_header;
  io->io_print_record          = io_ft_print_record;
  io->io_print_aggr_record     = io_ft_print_aggr_record;

  io->io_record_get_StartTS    = io_ft_record_get_StartTS;
  io->io_record_get_EndTS      = io_ft_record_get_EndTS;

  io->io_write_init            = io_ft_write_init;
  io->io_write_record          = io_ft_write_record;
  io->io_write_close           = io_ft_write_close;

  io->io_ctxt_destroy          = io_ft_ctxt_destroy;

  /* io->io_ctxt unused */

  return io;
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* I/O handler methods                                                      */
/*--------------------------------------------------------------------------*/

static
io_reader_t*
io_ft_read_init(io_ctxt_t* io_ctxt, int read_fd) {
  UNUSED(io_ctxt);

  struct ft_data* data = ft_init(read_fd);
  io_reader_t* io_reader = (io_reader_t*) data;

  return io_reader;
}

static
char*
io_ft_read_record(io_reader_t* io_reader) {
  return ftio_read(&io_reader->d.ft.io);
}

static
size_t
io_ft_read_get_field_offset(io_reader_t* io_reader, const char* field) {
  return io_ft_get_offset(field, &io_reader->d.ft.offsets);
}

static
size_t
io_ft_read_get_record_size(io_reader_t* read_ctxt) {
  return (size_t) read_ctxt->d.ft.rec_size;
}

static
int
io_ft_read_close(io_reader_t* io_reader) {
  ft_close(&io_reader->d.ft);

  return 0;
}

static
void
io_ft_print_debug_header(io_reader_t* io_reader) {
  ftio_header_print(&io_reader->d.ft.io, stdout, '#');
}

static
void
io_ft_print_header(io_reader_t* io_reader) {
  puts("\nStart             End               Sif   SrcIPaddress    SrcP  DIf   DstIPaddress    DstP    P Fl Pkts       Octets\n");
}

static
void
io_ft_print_record(io_reader_t* io_reader, char* record) {
  flow_print_record(&io_reader->d.ft, record);
}

static
void
io_ft_print_aggr_record(io_reader_t* io_reader,
                        struct aggr_record* aggr_record) {
  flow_print_group_record(&io_reader->d.ft, aggr_record);
}

static
uint64_t
io_ft_record_get_StartTS(io_reader_t* read_ctxt, char* record) {
    struct ft_data* dataformat = &read_ctxt->d.ft;

    uint32_t sysUpTime = *(u_int32_t*)(record + (dataformat->offsets).sysUpTime);
    uint32_t unix_secs = *(u_int32_t*)(record + (dataformat->offsets).unix_secs);
    uint32_t unix_nsecs = *(u_int32_t*)(record + (dataformat->offsets).unix_nsecs);
    uint32_t First = *(u_int32_t*)(record + (dataformat->offsets).First);

    struct fttime ft_start_ts = ftltime(sysUpTime,unix_secs,unix_nsecs,First);

    uint64_t start_ts_msecs = 1000LL * ft_start_ts.secs + ft_start_ts.msecs;

    return start_ts_msecs;
}

static
uint64_t
io_ft_record_get_EndTS(io_reader_t* read_ctxt, char* record) {
    struct ft_data* dataformat = &read_ctxt->d.ft;

    uint32_t sysUpTime = *(u_int32_t*)(record + (dataformat->offsets).sysUpTime);
    uint32_t unix_secs = *(u_int32_t*)(record + (dataformat->offsets).unix_secs);
    uint32_t unix_nsecs = *(u_int32_t*)(record + (dataformat->offsets).unix_nsecs);
    uint32_t Last = *(u_int32_t*)(record + (dataformat->offsets).Last);

    struct fttime ft_end_ts = ftltime(sysUpTime,unix_secs,unix_nsecs,Last);

    uint64_t end_ts_msecs = 1000LL * ft_end_ts.secs + ft_end_ts.msecs;

    return end_ts_msecs;
}

static
io_writer_t*
io_ft_write_init(io_reader_t* io_reader,
                 int          write_fd,
                 uint32_t     num_records) {
  struct ftio* ftio = get_ftio(&io_reader->d.ft, write_fd, num_records);
  if (ftio == NULL) return NULL;
  
  /* write the header to the output stream */
  if (ftio_write_header(ftio) < 0) {
    fterr_errx(1, "ftio_write_header(): failed");
  }
  
  return (io_writer_t*) ftio;
}

static
int
io_ft_write_record(io_writer_t* io_writer, char* record) {
  return ftio_write(&io_writer->d.ft, record);
}

static
int io_ft_write_close(io_writer_t* io_writer) {
  int rv = ftio_close(&io_writer->d.ft);

  free(io_writer);

  return rv;
}

static
void
io_ft_ctxt_destroy(io_ctxt_t* io_ctxt) {
}


/*--------------------------------------------------------------------------*/
/* Legacy methods                                                           */
/*--------------------------------------------------------------------------*/

static
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

static
size_t
io_ft_get_offset(const char * const name,
                 const struct fts3rec_offsets* const offsets) {

  /* IPFIX -> NFv5 mappings */
  #define CASEOFF_IPFIX(ipfix_ie, memb)       \
  if (strcmp(name, #ipfix_ie) == 0)           \
    return offsets->memb

  /* NFv5 only */
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
	CASEOFF_IPFIX(sourceTransportPort, srcport);
	CASEOFF_IPFIX(destinationTransportPort, dstport);
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
	CASEOFF(peer_nexthop);
	CASEOFF(router_sc);
	CASEOFF(src_tag);
	CASEOFF(dst_tag);
	CASEOFF(extra_pkts);
	CASEOFF(marked_tos);

  #undef CASEOFF_IPFIX
  #undef CASEOFF

  return -1;
}

static
void
ft_close(struct ft_data* data) {

  ftio_close(&data->io);

  if(data->fd)
    close(data->fd);
  free(data); data = NULL;
}

static
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

static
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

  struct tm* tm;
  time_t t_first_secs = aggr_record->start_ts_msec / 1000;
  u_long first_msecs = aggr_record->start_ts_msec % 1000;
  tm = localtime(&t_first_secs);

  printf("%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu ",
         (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
         (int)tm->tm_min, (int)tm->tm_sec, first_msecs);

  time_t t_last_secs = aggr_record->end_ts_msec / 1000;
  u_long last_msecs = aggr_record->end_ts_msec % 1000;
  tm = localtime(&t_last_secs);

  printf("%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu ",
         (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
         (int)tm->tm_min, (int)tm->tm_sec, last_msecs);

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

static
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
