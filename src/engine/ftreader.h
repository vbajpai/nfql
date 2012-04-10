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

#ifndef flowy_engine_ftreader_h
#define flowy_engine_ftreader_h

#include "base.h"
#include "flowy.h"

#include <fcntl.h>
#include <ftlib.h>

struct ft_data {
  int                             fd;
  struct ftio                     io;
  struct fts3rec_offsets          offsets;
  struct ftver                    version;
  u_int64_t                       xfield;
  int                             rec_size;
  struct record**                 recordset;
  int                             num_records;
};

struct record {
  char*                           record;
  bool                            if_filtered;
};


struct ft_data *
ft_open(int fd);

void 
ft_write(struct ft_data *data, int fd);

void 
ft_records_get_all(struct ft_data* data, int number, 
                   struct fts3rec_all *record);

u_int32_t *ft_records_get_unix_secs(struct ft_data* data, int number);
u_int32_t *ft_records_get_unix_nsecs(struct ft_data* data, int number);
u_int32_t *ft_records_get_sysUpTime(struct ft_data* data, int number);
u_int32_t *ft_records_get_exaddr(struct ft_data* data, int number);
u_int32_t *ft_records_get_srcaddr(struct ft_data* data, int number);
u_int32_t *ft_records_get_dstaddr(struct ft_data* data, int number);
u_int32_t *ft_records_get_nexthop(struct ft_data* data, int number);
u_int16_t *ft_records_get_input(struct ft_data* data, int number);
u_int16_t *ft_records_get_output(struct ft_data* data, int number);
u_int32_t *ft_records_get_dFlows(struct ft_data* data, int number);
u_int32_t *ft_records_get_dPkts(struct ft_data* data, int number);
u_int32_t *ft_records_get_dOctets(struct ft_data* data, int number);
u_int32_t *ft_records_get_First(struct ft_data* data, int number);
u_int32_t *ft_records_get_Last(struct ft_data* data, int number);
u_int16_t *ft_records_get_srcport(struct ft_data* data, int number);
u_int16_t *ft_records_get_dstport(struct ft_data* data, int number);
u_int8_t  *ft_records_get_prot(struct ft_data* data, int number);
u_int8_t  *ft_records_get_tos(struct ft_data* data, int number);
u_int8_t  *ft_records_get_tcp_flags(struct ft_data* data, int number);
u_int8_t  *ft_records_get_engine_type(struct ft_data* data, int number);
u_int8_t  *ft_records_get_engine_id(struct ft_data* data, int number);
u_int8_t  *ft_records_get_src_mask(struct ft_data* data, int number);
u_int8_t  *ft_records_get_dst_mask(struct ft_data* data, int number);
u_int16_t *ft_records_get_src_as(struct ft_data* data, int number);
u_int16_t *ft_records_get_dst_as(struct ft_data* data, int number);
u_int8_t  *ft_records_get_in_encaps(struct ft_data* data, int number);
u_int8_t  *ft_records_get_out_encaps(struct ft_data* data, int number);
u_int32_t *ft_records_get_peer_nexthop(struct ft_data* data, int number);
u_int32_t *ft_records_get_router_sc(struct ft_data* data, int number);
u_int32_t *ft_records_get_src_tag(struct ft_data* data, int number);
u_int32_t *ft_records_get_dst_tag(struct ft_data* data, int number);
u_int32_t *ft_records_get_extra_pkts(struct ft_data* data, int number);
u_int8_t  *ft_records_get_marked_tos(struct ft_data* data, int number);

void ft_close(struct ft_data* data);
void flow_print_record(struct ft_data *, char *);

#endif