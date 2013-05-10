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

#ifndef f_engine_io_h
#define f_engine_io_h

#include "pipeline.h"
#include "io-ft.h"
#include "ipfix.h"

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

typedef struct io_reader_s {
  union {
    struct ft_data         ft;
    struct ipfix_handler_s ipfix;
  } d;
} io_reader_t;

typedef struct io_writer_s {
  union {
    struct ftio     ft;
    ipfix_handler_t ipfix;
  } d;
} io_writer_t;

typedef struct io_ctxt_s {
  union {
    ipfix_templ_t* ipfix;
  } d;
} io_ctxt_t;

typedef struct io_data_s {
  char** recordset;
  size_t num_records;
} io_data_t;

typedef struct io_handler_s {
  io_reader_t*  (*io_read_init)(io_ctxt_t* io_ctxt, int read_fd);
  char*         (*io_read_record)(io_reader_t* read_ctxt);
  size_t        (*io_read_get_field_offset)(io_reader_t* read_ctxt, const char* field);
  size_t        (*io_read_get_record_size)(io_reader_t* read_ctxt);
  int           (*io_read_close)(io_reader_t* io_reader);

  void          (*io_print_header)(io_reader_t* io_reader);
  void          (*io_print_record)(io_reader_t* io_reader, char* record);
  void          (*io_print_aggr_record)(io_reader_t* io_reader,
                                        struct aggr_record* aggr_record);

  uint64_t      (*io_record_get_StartTS)(io_reader_t* read_ctxt, char* record);
  uint64_t      (*io_record_get_EndTS)(io_reader_t* read_ctxt, char* record);

  io_writer_t*  (*io_write_init)(io_reader_t* io_reader,
                                 int write_fd,
                                 uint32_t num_records);
  int           (*io_write_record)(io_writer_t* writer_ctxt, char* record);
  int           (*io_write_close)(io_writer_t* io_writer);

  void          (*io_handler_destroy)(io_ctxt_t* io_ctxt);

  io_ctxt_t ctxt;

} io_handler_t;

/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

/**
 * @brief Create IPFIX I/O handler
 *
 * @return I/O handler on success
 * @return NULL on failure
 */
io_handler_t*
ipfix_io_handler(ipfix_templ_t* ipfix);

/**
 * @brief Create flow-tools I/O handler
 *
 * @return I/O handler on success
 * @return NULL on failure
 */
io_handler_t*
ft_io_handler(void);

#endif // ! f_engine_io_h
