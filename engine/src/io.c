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

#include "io.h"

#include "io-ft.h"
#include "errorhandlers.h"

/*--------------------------------------------------------------------------*/
/* Local methods                                                            */
/*--------------------------------------------------------------------------*/

/* flow-tools adapter */
static void io_ft_handler_destroy(io_ctxt_t* io_ctxt);

/*--------------------------------------------------------------------------*/
/* Local implementation                                                     */
/*--------------------------------------------------------------------------*/

io_handler_t*
ft_io_handler(void) {
  io_handler_t* io = calloc(1, sizeof(io_handler_t));
  exitOn(io == NULL);

  io->io_read_init             = io_ft_read_init;
  io->io_read_record           = io_ft_read_record;
  io->io_read_get_field_offset = io_ft_read_get_field_offset;
  io->io_read_get_record_size  = io_ft_read_get_record_size;
  io->io_read_close            = io_ft_read_close;

  io->io_print_header          = io_ft_print_header;
  io->io_print_record          = io_ft_print_record;
  io->io_print_aggr_record     = io_ft_print_aggr_record;

  io->io_record_get_StartTS    = io_ft_record_get_StartTS;
  io->io_record_get_EndTS      = io_ft_record_get_EndTS;

  io->io_write_init            = io_ft_write_init;
  io->io_write_record          = io_ft_write_record;
  io->io_write_close           = io_ft_write_close;

  io->io_handler_destroy       = io_ft_handler_destroy;

  return io;
}

static
void
io_ft_handler_destroy(io_ctxt_t* io_ctxt) {
}
