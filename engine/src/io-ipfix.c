/*
 * Copyright 2013 Corneliu Prodescu <cprodescu@gmail.com>
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

#include "io-ipfix.h"

#include "io.h"
#include "ipfix-constants.h"
#include "errorhandlers.h"
#include "utils.h"

#include <assert.h>
#include <ftlib.h>

#define     FMT_BUF_SIZE    1024
#define     FMT_COL_SEP     "  "
/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef struct ipfix_ie_s {
  char              *name;
  size_t             offset;
  size_t             size;
  enum ipfix_ie_type type;
} ipfix_ie_t;

typedef struct ipfix_templ_s {
  ipfix_ie_t   *arr;
  size_t        len;
  size_t        next_offset;
} ipfix_templ_t;

struct debug_record {
    uint64_t        flowStartMilliseconds;          /*   0-  7 */
    uint64_t        flowEndMilliseconds;            /*   8- 15 */

    uint8_t         sourceIPv6Address[16];          /*  16- 31 */
    uint8_t         destinationIPv6Address[16];     /*  32- 47 */

    uint32_t        sourceIPv4Address;              /*  48- 51 */
    uint32_t        destinationIPv4Address;         /*  52- 55 */

    uint16_t        sourceTransportPort;            /*  56- 57 */
    uint16_t        destinationTransportPort;       /*  58- 59 */

    uint32_t        ipNextHopIPv4Address;           /*  60- 63 */
    uint8_t         ipNextHopIPv6Address[16];       /*  64- 79 */
    uint32_t        ingressInterface;               /*  80- 83 */
    uint32_t        egressInterface;                /*  84- 87 */

    uint64_t        packetDeltaCount;               /*  88- 95 */
    uint64_t        octetDeltaCount;                /*  96-103 */

    uint8_t         protocolIdentifier;             /*  104-105 */

    uint8_t         ipClassOfService;               /*  105-106 */
    uint8_t         tcpControlBits;                 /*  106-107 */

    uint8_t         paddingOctets[5];               /*  107-112 */
};

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

/* I/O handler                                                              */

static  struct  io_reader_s*  io_ipfix_read_init(struct io_ctxt_s* io_ctxt,
                                                 int read_fd);
static  char*   io_ipfix_read_record(struct io_reader_s* read_ctxt);
static  size_t  io_ipfix_read_get_field_offset(struct io_reader_s* read_ctxt,
                                               const char* field);
static  size_t  io_ipfix_read_get_record_size(struct io_reader_s* read_ctxt);
static  int     io_ipfix_read_close(struct io_reader_s* read_ctxt);

static  void    io_ipfix_print_header(struct io_reader_s* read_ctxt);
static  void    io_ipfix_print_debug_header(struct io_reader_s* read_ctxt);
static  void    io_ipfix_print_record(struct io_reader_s* read_ctxt,
                                      char* record);
static  void    io_ipfix_print_aggr_record(struct io_reader_s* read_ctxt,
                                           struct aggr_record* aggr_record);

static  uint64_t    io_ipfix_record_get_StartTS(struct io_reader_s* read_ctxt,
                                                char* record);
static  uint64_t    io_ipfix_record_get_EndTS(struct io_reader_s* read_ctxt,
                                              char* record);

static  const char* io_ipfix_get_format_suffix(void);
static  struct io_writer_s* io_ipfix_write_init(struct io_reader_s* read_ctxt,
                                                int write_fd,
                                                uint32_t num_records);
static int      io_ipfix_write_record(struct io_writer_s* write_ctxt,
                                      char* record);
static int      io_ipfix_write_close(struct io_writer_s* write_ctxt);
static void     io_ipfix_ctxt_destroy(struct io_ctxt_s* io_ctxt);

/* Field formatter                                                          */

static
void fmt_field_str(char* dest_str, size_t dest_size,
               char* data, enum ipfix_ie_type type);
static
int fmt_field_size(enum ipfix_ie_type type);

/* Template specification                                                   */

/**
 * @return retrieve IE ptr from template
 */
static
ipfix_ie_t*
ipfix_templ_get_ie(const ipfix_templ_t* templ,
                   const char * const ie_name);

/**
 * @return positive size in bytes on success, -1 on failure
 */
static
ssize_t
ipfix_ie_type_sizeof(enum ipfix_ie_type type);


/**
 * @return internal fibxuf template spec on success
 * @return NULL on failure
 */
static
fbInfoElementSpec_t*
ipfix_get_fb_info_elem_spec(const ipfix_templ_t* templ);


/*--------------------------------------------------------------------------*/
/* Implementation                                                           */
/*--------------------------------------------------------------------------*/

io_handler_t*
ipfix_io_handler(ipfix_templ_t* templ_spec) {

  io_handler_t* io = calloc(1, sizeof(io_handler_t));
  exitOn(io == NULL);

  io->io_read_init             = io_ipfix_read_init;
  io->io_read_record           = io_ipfix_read_record;
  io->io_read_get_field_offset = io_ipfix_read_get_field_offset;
  io->io_read_get_record_size  = io_ipfix_read_get_record_size;
  io->io_read_close            = io_ipfix_read_close;

  io->io_print_header          = io_ipfix_print_header;
  io->io_print_debug_header    = io_ipfix_print_debug_header;
  io->io_print_record          = io_ipfix_print_record;
  io->io_print_aggr_record     = io_ipfix_print_aggr_record;

  io->io_record_get_StartTS    = io_ipfix_record_get_StartTS;
  io->io_record_get_EndTS      = io_ipfix_record_get_EndTS;

  io->io_get_format_suffix     = io_ipfix_get_format_suffix;
  io->io_write_init            = io_ipfix_write_init;
  io->io_write_record          = io_ipfix_write_record;
  io->io_write_close           = io_ipfix_write_close;

  io->io_ctxt_destroy          = io_ipfix_ctxt_destroy;

  /* initialize IPFIX I/O context */
  struct ipfix_ctxt_s* ipfix = &io->ctxt.d.ipfix;

  ipfix->templ_spec  = templ_spec;

  ipfix->fb_info_model = fbInfoModelAlloc();
  exitOn(ipfix->fb_info_model == NULL);

  ipfix->fb_templ = fbTemplateAlloc(ipfix->fb_info_model);
  exitOn(ipfix->fb_templ == NULL);

  fbInfoElementSpec_t* ie_spec = ipfix_get_fb_info_elem_spec(templ_spec);
  GError* err = NULL;

  exitOn(ie_spec == NULL);
  if(fbTemplateAppendSpecArray(ipfix->fb_templ,
                               ie_spec,
                               0xffffffff,
                               &err) == FALSE) {
     errExit("fbTemplateAppendSpecArray(..): %s", err->message);
  }

  free(ie_spec);

  return io;
}

/* Template specification                                                   */

ipfix_templ_t*
ipfix_templ_new(void) {
  ipfix_templ_t* ipfix_templ = calloc(1, sizeof(ipfix_templ_t));
  if (ipfix_templ == NULL) return NULL;

#define REGISTER(ie_name)                                                   \
  do {                                                                      \
    if (ipfix_templ_ie_register(ipfix_templ, ie_name) < 0) {                \
      free(ipfix_templ);                                                    \
      return NULL;                                                          \
    }                                                                       \
  } while (0)

  /* absolute timestamps */
  REGISTER("flowStartMilliseconds");
  REGISTER("flowEndMilliseconds");

  /* 4-tuple */
  //REGISTER("sourceIPv6Address");
  //REGISTER("destinationIPv6Address");
  REGISTER("sourceIPv4Address");
  REGISTER("destinationIPv4Address");
  REGISTER("sourceTransportPort");
  REGISTER("destinationTransportPort");

  /* Router interface information */
  //REGISTER("ipNextHopIPv4Address");
  //REGISTER("ipNextHopIPv6Address");
  REGISTER("ingressInterface");
  REGISTER("egressInterface");

  /* Counters */
  REGISTER("packetDeltaCount");
  REGISTER("octetDeltaCount");
  // REGISTER("deltaFlowCount"); // draft-ietf-ipfix-a9n-08 - not in fixbuf

  /* Protocol; sensor information */
  REGISTER("protocolIdentifier");

  /* Flags */
  REGISTER("ipClassOfService");
  REGISTER("tcpControlBits");

#undef REGISTER

  return ipfix_templ;
}

int
ipfix_templ_ie_register(ipfix_templ_t* templ,
                        const char * const ie_name) {
  /* do not insert duplicate IEs */
  if (ipfix_templ_get_ie(templ, ie_name) != NULL) return 0;

  enum ipfix_ie_type ie_type = get_ipfix_type(ie_name);
  if (ie_type == IETYPE_UNKNOWN) return -1;

  /* compute offset - needs to be aligned to be type */
  size_t size = ipfix_ie_type_sizeof(ie_type);

  size_t rest = templ->next_offset % size;
  if (rest != 0) { /* padding required */
    templ->next_offset += size - rest;
  }

  /* add new ie to array */
  templ->arr = realloc(templ->arr, sizeof(ipfix_ie_t) * (templ->len + 1));
  if (templ->arr == NULL) return -1;
  ipfix_ie_t *new_ie = &templ->arr[templ->len];
  new_ie->name   = strdup(ie_name);
  exitOn(new_ie->name == NULL);
  new_ie->offset = templ->next_offset;
  new_ie->size   = size;
  new_ie->type   = ie_type;

  assert(new_ie->offset % new_ie->size == 0);

  /* store new length and next_offset */
  templ->next_offset += size;
  templ->len++;

  return 0;
}


ssize_t
ipfix_templ_get_ie_offset(const ipfix_templ_t* templ,
                          const char * const ie_name) {
  ipfix_ie_t* ie = ipfix_templ_get_ie(templ, ie_name);
  if (ie == NULL) return -1;

  return ie->offset;
}

size_t
ipfix_templ_get_rec_size(const ipfix_templ_t* templ) {

  /* The size is the next_offset, rounded to 8byte */
  size_t size = templ->next_offset;
  size += 7;
  size >>= 3;
  size <<= 3;

  return size;
}

void
ipfix_templ_free(ipfix_templ_t* ipfix_templ) {
  size_t i;
  for (i = 0; i < ipfix_templ->len; i++) {
    free(ipfix_templ->arr[i].name);
  }
  free(ipfix_templ->arr);
  free(ipfix_templ);
}


/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

/* I/O handler                                                              */

static
struct  io_reader_s*
io_ipfix_read_init(struct io_ctxt_s* io_ctxt,
                   int read_fd) {
  struct ipfix_ctxt_s* ipfix = &io_ctxt->d.ipfix;

  /* allocate ipfix_reader space */
  struct ipfix_reader_s* read_ctxt = calloc(1, sizeof(struct ipfix_reader_s));
  exitOn(read_ctxt == NULL);

  /* create fixbuf session */
  fbSession_t* session = fbSessionAlloc(ipfix->fb_info_model);
  exitOn(session == NULL);

  uint16_t templateID = fbSessionAddTemplate(session, TRUE, FB_TID_AUTO,
                                             ipfix->fb_templ, &read_ctxt->err);
  exitOn(templateID == 0);

  /* create fixbuf collector */
  read_ctxt->fp = fdopen(read_fd, "r");
  exitOn(read_ctxt->fp == NULL);

  fbCollector_t* collector = fbCollectorAllocFP(NULL, read_ctxt->fp);
  exitOn(collector == NULL);

  /* create fixbuf fbuf */
  read_ctxt->fbuf = fBufAllocForCollection(session, collector);
  exitOn(read_ctxt->fbuf == NULL);

  exitOn(fBufSetInternalTemplate(read_ctxt->fbuf,
                                 templateID,
                                 &read_ctxt->err) == FALSE);

  /* save info_model, templ_spec, fb_templ */
  read_ctxt->templ_spec = ipfix->templ_spec;
  read_ctxt->fb_info_model = ipfix->fb_info_model;
  read_ctxt->fb_templ = ipfix->fb_templ;

  /* save rec_size, first_off, last_off */
  read_ctxt->rec_size =
        ipfix_templ_get_rec_size(read_ctxt->templ_spec);
  read_ctxt->first_off = ipfix_templ_get_ie_offset(ipfix->templ_spec,
                                                   "flowStartMilliseconds");
  read_ctxt->last_off = ipfix_templ_get_ie_offset(ipfix->templ_spec,
                                                  "flowEndMilliseconds");

  /* alloc rec_buf */
  read_ctxt->rec_buf = malloc(read_ctxt->rec_size);

  return (struct io_reader_s*) read_ctxt;
}

static  char*   io_ipfix_read_record(struct io_reader_s* _read_ctxt) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  size_t rec_size = read_ctxt->rec_size;

  if (fBufNext(read_ctxt->fbuf,
               read_ctxt->rec_buf, &rec_size,
               &read_ctxt->err)) {

    assert(rec_size == read_ctxt->rec_size);

    return (char*)read_ctxt->rec_buf;
  } else if (read_ctxt->err->code == FB_ERROR_EOF) {

  } else {
    errExit("%s", read_ctxt->err->message);
  }

  return NULL;
}

static  size_t  io_ipfix_read_get_field_offset(struct io_reader_s* _read_ctxt,
                                               const char* field) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  ssize_t offset = ipfix_templ_get_ie_offset(read_ctxt->templ_spec, field);
  exitOn(offset == -1);

  return (size_t) offset;
}

static  size_t  io_ipfix_read_get_record_size(struct io_reader_s* _read_ctxt) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  return read_ctxt->rec_size;
}

static  int     io_ipfix_read_close(struct io_reader_s* _read_ctxt) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  FILE* fp = read_ctxt->fp;

  fBufFree(read_ctxt->fbuf);
  free(read_ctxt->rec_buf);
  free(read_ctxt);

  return fclose(fp);
}

static  void    io_ipfix_print_header(struct io_reader_s* read_ctxt) {
  size_t len = read_ctxt->d.ipfix.templ_spec->len;
  struct ipfix_ie_s* ies = read_ctxt->d.ipfix.templ_spec->arr;

  size_t i;
  for (i = 0; i < len; i++) {
    if (strcmp(ies[i].name, "paddingOctets") != 0) {
      int fmt_size = fmt_field_size(ies[i].type) + sizeof(FMT_COL_SEP) - 1;

      printf("%*.*s" FMT_COL_SEP, fmt_size, fmt_size, ies[i].name);
    }
  }
  printf("\n");
}

static  void    io_ipfix_print_debug_header(struct io_reader_s* read_ctxt) {
}

static  void    io_ipfix_print_record(struct io_reader_s* read_ctxt,
                                      char* record) {
  size_t len = read_ctxt->d.ipfix.templ_spec->len;
  struct ipfix_ie_s* ies = read_ctxt->d.ipfix.templ_spec->arr;

  size_t i;
  for (i = 0; i < len; i++) {
    if (strcmp(ies[i].name, "paddingOctets") == 0) { /* ignore */
    } else {
      char buf[FMT_BUF_SIZE];
      int col_size = fmt_field_size(ies[i].type) + sizeof(FMT_COL_SEP) - 1;
      char *data = record + ies[i].offset;
      fmt_field_str(buf, sizeof(buf), data, ies[i].type);
      printf("%*s" FMT_COL_SEP, col_size, buf);
    }
  }
  printf("\n");
}

static  void    io_ipfix_print_aggr_record(struct io_reader_s* read_ctxt,
                                           struct aggr_record* aggr_record) {
  size_t len = read_ctxt->d.ipfix.templ_spec->len;
  struct ipfix_ie_s* ies = read_ctxt->d.ipfix.templ_spec->arr;

  size_t i;
  for (i = 0; i < len; i++) {
    if (strcmp(ies[i].name, "paddingOctets") == 0) { /* ignore */
    } else {
      char buf[FMT_BUF_SIZE];
      int col_size = fmt_field_size(ies[i].type) + sizeof(FMT_COL_SEP) - 1;
      char* data;

      if (strcmp(ies[i].name, "flowStartMilliseconds") == 0) { /* print aggr */
        data = (char*) &aggr_record->start_ts_msec;
      } else if (strcmp(ies[i].name, "flowEndMilliseconds") == 0) { /* print aggr */
        data = (char*) &aggr_record->end_ts_msec;
      } else {
        data = aggr_record->aggr_record + ies[i].offset;
      }

      fmt_field_str(buf, sizeof(buf), data, ies[i].type);
      printf("%*s" FMT_COL_SEP, col_size, buf);
    }
  }
  printf("\n");
}

static  uint64_t    io_ipfix_record_get_StartTS(struct io_reader_s* _read_ctxt,
                                                char* record) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  return *(uint64_t*)(record + read_ctxt->first_off);
}

static  uint64_t    io_ipfix_record_get_EndTS(struct io_reader_s* _read_ctxt,
                                              char* record) {
  struct ipfix_reader_s* read_ctxt = &_read_ctxt->d.ipfix;

  return *(uint64_t*)(record + read_ctxt->last_off);
}

static  const char* io_ipfix_get_format_suffix(void) {
  return "ipfix";
}

static  struct io_writer_s* io_ipfix_write_init(struct io_reader_s* _read_ctxt,
                                                int write_fd,
                                                uint32_t num_records) {
  UNUSED(num_records);

  struct ipfix_reader_s* ipfix = &_read_ctxt->d.ipfix;

  /* allocate ipfix_reader space */
  struct ipfix_writer_s* writer_ctxt = calloc(1, sizeof(struct ipfix_writer_s));

  /* create fixbuf session */
  fbSession_t* session = fbSessionAlloc(ipfix->fb_info_model);
  exitOn(session == NULL);

  uint16_t intTemplateID = fbSessionAddTemplate(session,
                                                /* internal = */ TRUE,
                                                FB_TID_AUTO,
                                                ipfix->fb_templ,
                                                &writer_ctxt->err);
  exitOn(intTemplateID == 0);
  uint16_t extTemplateID = fbSessionAddTemplate(session,
                                                /* internal = */ FALSE,
                                                FB_TID_AUTO,
                                                ipfix->fb_templ,
                                                &writer_ctxt->err);
  exitOn(extTemplateID == 0);

  /* create fixbuf exporter */
  writer_ctxt->fp = fdopen(write_fd, "w");
  exitOn(writer_ctxt->fp == NULL);

  fbExporter_t* exporter = fbExporterAllocFP(writer_ctxt->fp);
  exitOn(exporter == NULL);

  /* create fixbuf fbuf */
  writer_ctxt->fbuf = fBufAllocForExport(session, exporter);
  exitOn(writer_ctxt->fbuf == NULL);
  if (fBufSetInternalTemplate(writer_ctxt->fbuf,
                                 intTemplateID,
                                 &writer_ctxt->err) == FALSE) {
    puts(writer_ctxt->err->message);
  }
  if (fBufSetExportTemplate(writer_ctxt->fbuf,
                               extTemplateID,
                               &writer_ctxt->err) == FALSE) {
    puts(writer_ctxt->err->message);
  }

  exitOn(fbSessionExportTemplates(session, &writer_ctxt->err) == FALSE);

  /* save rec_size */
  writer_ctxt->rec_size = ipfix->rec_size;

  return (struct io_writer_s*) writer_ctxt;
}

static int      io_ipfix_write_record(struct io_writer_s* _writer_ctxt,
                                      char* record) {
  struct ipfix_writer_s* writer_ctxt = &_writer_ctxt->d.ipfix;

  if (fBufAppend(writer_ctxt->fbuf,
                 (uint8_t*)record, writer_ctxt->rec_size,
                 &writer_ctxt->err) == FALSE) {
    return -1;
  } else {
    return 0;
  }
}

static int      io_ipfix_write_close(struct io_writer_s* _writer_ctxt) {
  struct ipfix_writer_s* writer_ctxt = &_writer_ctxt->d.ipfix;
  FILE* fp = writer_ctxt->fp;

  GError *err;
  exitOn(fBufEmit(writer_ctxt->fbuf, &err) == FALSE);
  fBufFree(writer_ctxt->fbuf);
  free(writer_ctxt);

  return fclose(fp);
}

static
void
io_ipfix_ctxt_destroy(struct io_ctxt_s* io_ctxt) {
  struct ipfix_ctxt_s* ipfix = &io_ctxt->d.ipfix;

  ipfix_templ_free(ipfix->templ_spec);
  fbInfoModelFree(ipfix->fb_info_model);
  /* fb_templ is reference counted across fbSessions and freed with
     the last session */
}

/* Field formatter                                                          */

static
void fmt_field_str(char* str, size_t size,
                   char* data, enum ipfix_ie_type type) {
  switch (type) {
    case IETYPE_UNSIGNED8:
      snprintf(str, size, "%"PRIu8, *(uint8_t*) data);
      break;

    case IETYPE_UNSIGNED16:
      snprintf(str, size, "%"PRIu16, *(uint16_t*)data);
      break;

    case IETYPE_UNSIGNED32:
      snprintf(str, size, "%"PRIu32, *(uint32_t*)data);
      break;

    case IETYPE_UNSIGNED64:
      snprintf(str, size, "%"PRIu64, *(uint64_t*)data);
      break;

    case IETYPE_IPV4ADDRESS: {
        char fmt_buf[64];
        fmt_ipv4(fmt_buf, *(uint32_t*)data, FMT_PAD_LEFT);
        snprintf(str, size, "%s", fmt_buf);
        break;
    }

    case IETYPE_DATETIMEMILLISECONDS: {
      uint64_t time_msecs = *(uint64_t*)data;
      time_t secs = time_msecs / 1000;
      struct tm *tm = localtime(&secs);
      snprintf(str, size,
               "%-2.2d%-2.2d.%-2.2d:%-2.2d:%-2.2d.%-3.3lu",
               (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour,
               (int)tm->tm_min, (int)tm->tm_sec, (u_long)time_msecs % 1000);
      break;
    }
      /*
      case IETYPE_IPV6ADDRESS: {
        char fmt_buf[FMT_BUF_SIZE];
        uint8_t* addr = (uint8_t*)(record + ies[i].offset);
        snprintf(fmt_buf, sizeof(fmt_buf),
    "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ",
                (int)addr[0], (int)addr[1],
                (int)addr[2], (int)addr[3],
                (int)addr[4], (int)addr[5],
                (int)addr[6], (int)addr[7],
                (int)addr[8], (int)addr[9],
                (int)addr[10], (int)addr[11],
                (int)addr[12], (int)addr[13],
                (int)addr[14], (int)addr[15]);
        printf("%18s ", fmt_buf);
        break;
      }

      */
    default:
      snprintf(str, size, "no_print_fmt");
  }
}

static
int fmt_field_size(enum ipfix_ie_type type) {
  switch(type) {
    case IETYPE_UNSIGNED8: return 3;
    case IETYPE_UNSIGNED16: return 5;
    case IETYPE_UNSIGNED32: return 10;
    case IETYPE_UNSIGNED64: return 18;

    case IETYPE_IPV4ADDRESS: return 15;

    case IETYPE_DATETIMEMILLISECONDS: return 18;
    /*
    case IETYPE_DATETIMESECONDS: return 14;
    case IETYPE_DATETIMEMICROSECONDS:
    case IETYPE_DATETIMENANOSECONDS:
    case IETYPE_IPV6ADDRESS: return 39;
    */
    default: return 12;
  }
}

/* Template specification                                                   */

static
ipfix_ie_t*
ipfix_templ_get_ie(const ipfix_templ_t* templ,
                   const char * const ie_name) {
  int i;

  for (i = 0; i < templ->len; i++) {
    if (strcmp(ie_name, templ->arr[i].name) == 0) {
      return &templ->arr[i];
    }
  }

  return NULL;
}

static
ssize_t
ipfix_ie_type_sizeof(enum ipfix_ie_type type) {
  switch (type) {
    case IETYPE_UNSIGNED64:
      return 8;
    case IETYPE_UNSIGNED8:
      return 1;
    case IETYPE_FLOAT64:
      return 8;
    case IETYPE_UNSIGNED16:
      return 2;
    case IETYPE_UNSIGNED32:
      return 4;
    case IETYPE_IPV6ADDRESS:
      return 16;
    case IETYPE_IPV4ADDRESS:
      return 4;
    case IETYPE_DATETIMEMICROSECONDS:
    case IETYPE_DATETIMEMILLISECONDS:
    case IETYPE_DATETIMENANOSECONDS:
      return 8;
    case IETYPE_DATETIMESECONDS:
      return 4;
    case IETYPE_MACADDRESS:
      return 6;

    // Error
    case IETYPE_UNKNOWN:
    case IETYPE_STRING:
    case IETYPE_OCTETARRAY:
    case IETYPE_BOOLEAN:
    case IETYPE_SUBTEMPLATEMULTILIST:
    case IETYPE_BASICLIST:
    case IETYPE_SUBTEMPLATELIST:
      return -1;
  }

  return -1;
}

static
fbInfoElementSpec_t*
ipfix_get_fb_info_elem_spec(const ipfix_templ_t* templ) {
  fbInfoElementSpec_t* spec = NULL;
  ssize_t spec_size = 0;
  ssize_t curr_offset = 0;
  int i;

#define __fbtempl_add_elem(ie_name, size_override)                          \
  do {                                                                      \
    /* extend fbInfoElementSpec_st array */                                 \
    spec_size++;                                                            \
    fbInfoElementSpec_t *new_spec =                                         \
        realloc(spec, spec_size * sizeof(fbInfoElementSpec_t));             \
    if (new_spec == NULL) goto fail;                                        \
    spec = new_spec;                                                        \
    fbInfoElementSpec_t* new_elem = &spec[spec_size-1];                     \
    memset(new_elem, 0, sizeof(fbInfoElementSpec_t));                       \
    new_elem->name = ie_name;                                               \
    new_elem->len_override = size_override;                                 \
  } while (0)

#define _fbtempl_add_padding(padding_size)                                  \
  __fbtempl_add_elem("paddingOctets", padding_size)

#define _fbtempl_add(ie_name)                                               \
  __fbtempl_add_elem(ie_name, 0)


  for (i = 0; i < templ->len; i++) {
    ipfix_ie_t* ie = &templ->arr[i];

    if (ie->offset != curr_offset) { /* padding required */
      ssize_t padding_size = ie->offset - curr_offset;
      assert(padding_size > 0 && padding_size < 64);

      _fbtempl_add_padding(padding_size);
      curr_offset += padding_size;
    }

    _fbtempl_add(ie->name);

    curr_offset += ie->size;
  }

  /* end of struct padding to align to 8byte boundary */
  if (curr_offset % 8 != 0) {
    _fbtempl_add_padding(8 - curr_offset % 8);
  }

  /* Add final element - use padding spec element and replace it with
     the NULL one for convenience */
  fbInfoElementSpec_t fb_iespec_null = FB_IESPEC_NULL;
  _fbtempl_add_padding(0);
  memcpy(&spec[spec_size-1], &fb_iespec_null, sizeof(fb_iespec_null));

#undef __fbtempl_add_elem
#undef _fbtempl_add_padding
#undef _fbtempl_add

  return spec;

fail:
  free(spec);
  return NULL;
}
