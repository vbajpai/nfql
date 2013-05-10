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

#include <assert.h>

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef struct ipfix_ie_s {
  char              *name;
  size_t             offset;
  size_t             size;
} ipfix_ie_t;

typedef struct ipfix_templ_s {
  ipfix_ie_t   *arr;
  size_t        len;
  size_t        next_offset;
} ipfix_templ_t;


/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

static void io_ipfix_ctxt_destroy(io_ctxt_t* io_ctxt);
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
#if 0
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

  io->io_write_init            = io_ipfix_write_init;
  io->io_write_record          = io_ipfix_write_record;
  io->io_write_close           = io_ipfix_write_close;
#endif
  io->io_ctxt_destroy       = io_ipfix_ctxt_destroy;

  io->ctxt.d.ipfix.templ_spec = templ_spec;
  // TODO init info_model

  return io;
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/


static
void
io_ipfix_ctxt_destroy(io_ctxt_t* io_ctxt) {
  // TODO destroy fb info elem
}
/*--------------------------------------------------------------------------*/
/* I/O handler                                                              */
/*--------------------------------------------------------------------------*/

struct io_reader_s*
io_ipfix_read_init(struct io_ctxt_s* io_ctxt, int read_fd) {
  // TODO
  return NULL;
}

/*--------------------------------------------------------------------------*/
/* Template specification                                                   */
/*--------------------------------------------------------------------------*/

ipfix_templ_t*
ipfix_templ_new(void) {
  return calloc(1, sizeof(ipfix_templ_t));
}

int
ipfix_templ_ie_register(ipfix_templ_t* templ,
                        char * const ie_name) {
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
  new_ie->name   = ie_name;
  new_ie->offset = templ->next_offset;
  new_ie->size   = size;

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

/*--------------------------------------------------------------------------*/
/* Reader                                                                   */
/*--------------------------------------------------------------------------*/

struct ipfix_reader_s*
ipfix_init_read(FILE* fp,
                ipfix_templ_t* templ) {
  ipfix_templ_ie_register(templ, "tcpSourcePort");

  struct ipfix_reader_s* ipfix = calloc(1, sizeof(struct ipfix_reader_s));
  exitOn(ipfix == NULL);

  ipfix->info_model = fbInfoModelAlloc();
  exitOn(ipfix->info_model == NULL);

  ipfix->session = fbSessionAlloc(ipfix->info_model);
  exitOn(ipfix->session == NULL);

  ipfix->collector = fbCollectorAllocFP(NULL, fp);
  exitOn(ipfix->collector == NULL);

  ipfix->fbuf = fBufAllocForCollection(ipfix->session, ipfix->collector);
  exitOn(ipfix->fbuf == NULL);

  fbTemplate_t *fb_templ = fbTemplateAlloc(ipfix->info_model);
  exitOn(fb_templ == NULL);
  fbInfoElementSpec_t* ie_spec = ipfix_get_fb_info_elem_spec(templ);
  exitOn(ie_spec == NULL);
  exitOn(fbTemplateAppendSpecArray(fb_templ,
                                   ie_spec,
                                   0xffffffff,
                                   &ipfix->err) == FALSE);
  free(ie_spec);

  uint16_t templateID = fbSessionAddTemplate(ipfix->session, TRUE, FB_TID_AUTO,
                                             fb_templ, &ipfix->err);
  exitOn(templateID == 0);
  exitOn(fBufSetInternalTemplate(ipfix->fbuf,
                                 templateID,
                                 &ipfix->err) == FALSE);

  uint8_t raw_buf[1024];
  size_t  raw_buf_size = sizeof(raw_buf);
  printf("START\n");
  int records = 0;
  while (fBufNext(ipfix->fbuf, raw_buf, &raw_buf_size, &ipfix->err)) {
      records++;
      printf("Read %d bytes\n", (int) raw_buf_size);
      raw_buf_size = sizeof(raw_buf);
  }
  printf("DONE %d records\n", records);

  fBufFree(ipfix->fbuf);

  return ipfix;
}

void
ipfix_ctxt_destroy(struct ipfix_ctxt_s* ipfix) {
  fbInfoModelFree(ipfix->info_model);
  free(ipfix->templ_spec);
}

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

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
      return 1;
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

  /* FIXME end of struct padding may be required */

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
