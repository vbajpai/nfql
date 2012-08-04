/*
 * Copyright 2012 Vaibhav Bajpai <contact@vaibhavbajpai.com>
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

#include "ftwriter.h"

int
get_fd(char* filename) {

  int out_fd = 1;

  if ((out_fd = open(filename,  O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1){
    fterr_err(1, "open(%s)", filename);
    return -1;
  }

  struct stat sb;
  if (fstat(out_fd, &sb) == -1) {
    fterr_err(1, "fstat(%s)", filename);
    return -1;
  }

  return out_fd;
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
