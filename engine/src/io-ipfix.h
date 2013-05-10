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

#ifndef f_engine_io_ipfix_h
#define f_engine_io_ipfix_h

#include "pipeline.h"

#include <fixbuf/public.h>
#include <glib.h>

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

struct io_ctxt_s;
struct ipfix_templ_s;
typedef struct ipfix_templ_s ipfix_templ_t;

struct ipfix_ctxt_s {
  struct ipfix_templ_s* templ_spec;
  fbInfoModel_t* info_model;
};

struct ipfix_reader_s {
  fbInfoModel_t *info_model;
  fbSession_t   *session;
  fbCollector_t *collector;
  fBuf_t        *fbuf;
  GError        *err;
};

struct ipfix_writer_s {
  fbInfoModel_t *info_model;
  fbSession_t   *session;
  fbCollector_t *collector;
  fBuf_t        *fbuf;
  GError        *err;
};

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

/**
 * @brief Create IPFIX I/O handler
 *
 * The template is owned by the I/O handler and will be destroyed when
 * io_ctxt_destroy is called.
 *
 * @return I/O handler on success
 * @return NULL on failure
 */
struct io_handler_s*
ipfix_io_handler(ipfix_templ_t* ipfix);

/*--------------------------------------------------------------------------*/
/* Template specification                                                   */
/*--------------------------------------------------------------------------*/

/**
 * @return ipfix_templ_t handle on success
 * @return NULL on failure
 *
 * The caller owns this object and needs to pass it to ipfix_templ_free() once
 * it is done using it.
 */
ipfix_templ_t*
ipfix_templ_new(void);

/**
 * @return 0 on success, -1 on failure
 */
int
ipfix_templ_ie_register(ipfix_templ_t* ipfix_templ,
                        const char * const ie_name);

void
ipfix_templ_free(ipfix_templ_t* ipfix_templ);

#endif
