/*
 * Copyright 2013 Corneliu Prodescu <cprodescu@gmail.com>
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

#ifndef f_engine_io_ft_h
#define f_engine_io_ft_h

#include "base.h"
#include "pipeline.h"

#include <ftlib.h>

/*--------------------------------------------------------------------------*/
/* Type declarations                                                        */
/*--------------------------------------------------------------------------*/

struct ft_data {
  int                             fd;
  struct ftio                     io;
  struct fts3rec_offsets          offsets;
  struct ftver                    version;
  u_int64_t                       xfield;
  int                             rec_size;
  char**                          recordset;
  size_t                          num_records;
};

struct io_ctxt_t;
typedef struct io_ctxt_s io_ctxt_t;
struct io_reader_s;
typedef struct io_reader_s io_reader_t;
struct io_writer_s;
typedef struct io_writer_s io_writer_t;


/*--------------------------------------------------------------------------*/
/* Methods                                                                  */
/*--------------------------------------------------------------------------*/

/* I/O handler methods */
io_reader_t* io_ft_read_init(io_ctxt_t* io_ctxt, int read_fd);
char*        io_ft_read_record(io_reader_t* io_reader);
size_t       io_ft_read_get_field_offset(io_reader_t* io_reader,
                                         const char* field);
size_t       io_ft_read_get_record_size(io_reader_t* read_ctxt);
int          io_ft_read_close(io_reader_t* io_reader);

void         io_ft_print_header(io_reader_t* io_reader);
void         io_ft_print_record(io_reader_t* io_reader, char* record);
void         io_ft_print_aggr_record(io_reader_t* io_reader,
                                     struct aggr_record* aggr_record);

io_writer_t* io_ft_write_init(io_reader_t* io_reader,
                              int write_fd,
                              uint32_t num_records);
int          io_ft_write_record(io_writer_t* io_writer, char* record);
int          io_ft_write_close(io_writer_t* io_writer);


/* Legacy methods */

struct ft_data* ft_init(int fsock);
size_t io_ft_get_offset(const char * const name,
                        const struct fts3rec_offsets* const offsets);
void ft_close(struct ft_data* data);
void flow_print_record(struct ft_data *, char *);
void flow_print_group_record(struct ft_data *data, struct aggr_record* aggr_record);
struct ftio* get_ftio(struct ft_data* const dataformat,
                      int out_fd,
                      uint32_t total_flows);

#endif
