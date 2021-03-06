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
#include "ipfix-constants.h"

#include <fixbuf/public.h>
#include <glib.h>

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

struct ipfix_ctxt_s {
  struct ipfix_templ_s* templ_spec;
  fbInfoModel_t*    fb_info_model;
  fbTemplate_t*     fb_templ;
  pthread_mutex_t*  fb_mutex;
};

struct ipfix_reader_s {
  struct ipfix_templ_s* templ_spec;
  fbInfoModel_t* fb_info_model;
  fbTemplate_t*  fb_templ;
  FILE*          fp;
  fBuf_t*        fbuf;
  GError*        err;
  uint8_t*       rec_buf;
  size_t         rec_size;
  size_t         first_off;
  size_t         last_off;
  pthread_mutex_t*  fb_mutex;
};

struct ipfix_writer_s {
  fBuf_t        *fbuf;
  GError        *err;
  FILE          *fp;
  size_t         rec_size;
  pthread_mutex_t*  fb_mutex;
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

/**
 * @return offset of the ie within the record on success
 * @return -1 on failure
 */
ssize_t
ipfix_templ_get_ie_offset(const ipfix_templ_t* templ,
                          const char * const ie_name);

size_t
ipfix_templ_get_rec_size(const ipfix_templ_t* templ);

/**
 * Free the ipfix_templ
 */
void
ipfix_templ_free(ipfix_templ_t* ipfix_templ);

#endif
