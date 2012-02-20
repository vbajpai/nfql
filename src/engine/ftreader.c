/*
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

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "ftreader.h"

struct ft_data *ft_open(int fd)
{
    int ret;
    struct ft_data *data;
    char *record;

    data = (struct ft_data *)calloc(1, sizeof(struct ft_data));

    data->fd = fd;

    ret = ftio_init(&data->io, data->fd, FT_IO_FLAG_READ);
    if (ret < 0) {
        perror("ftio_init failed");
        return NULL;
    }

    ftio_get_ver(&data->io, &data->version);
    data->xfield = ftio_xfield(&data->io);
    fts3rec_compute_offsets(&data->offsets, &data->version);
    data->rec_size = ftio_rec_size(&data->io);

    /*
     * TODO: optimize the reallocs here (eg by doubling the space every time
     *       one runs out of it)
     *
     * TODO: maybe allocate everything in one big chunk for faster iteration
     *
     */

    while ((record = ftio_read(&data->io)) != NULL) {
        data->num_records++;
        data->records = (char **)realloc(data->records, sizeof(char *)*data->num_records);
        data->records[data->num_records-1] = (char *)malloc(data->rec_size);
        memcpy(data->records[data->num_records-1], record, data->rec_size);
    }

    return data;
}

void ft_write(struct ft_data *data, int outfd)
{
    struct ftset ftset;
    int ret, i;
    struct ftio ftio_out;

    ftset_init(&ftset, 0);

    ftset.comments = ftio_get_comment(&data->io); // TODO: make configureable
    ftset.byte_order = FT_HEADER_LITTLE_ENDIAN; // TODO: make configureable
    ftset.z_level = 6; // from 0-9 TODO: make configureable

    ret = ftio_init(&ftio_out, outfd, FT_IO_FLAG_WRITE | ((ftset.z_level) ? FT_IO_FLAG_ZINIT : 0));
    if (ret < 0) {
        perror("ftio_init() failed");
        return;
    }

    ftio_set_byte_order(&ftio_out, ftset.byte_order);
    ftio_set_z_level(&ftio_out, ftset.z_level);
    ftio_set_streaming(&ftio_out, 0);
    ftio_set_debug(&ftio_out, 0); // TODO: make configureable

    ftio_set_preloaded(&ftio_out, 1);
    ftio_set_cap_time(&ftio_out, ftio_get_cap_start(&data->io), ftio_get_cap_end(&data->io));
    ftio_set_flows_count(&ftio_out, data->num_records);
    ftio_set_corrupt(&ftio_out, ftio_get_corrupt(&data->io));
    ftio_set_lost(&ftio_out, ftio_get_lost(&data->io));

    ret = ftio_set_comment(&ftio_out, ftset.comments);
    ret = ftio_set_ver(&ftio_out, &data->version);
    ret = ftio_write_header(&ftio_out);

    for (i = 0; i < data->num_records; i++) {
        ret = ftio_write(&ftio_out, data->records[i]);
    }

    ret = ftio_close(&ftio_out);
    close(outfd);
}

void ft_records_get_all(struct ft_data* data, int number, struct fts3rec_all *record)
{
    record->unix_secs = ft_records_get_unix_secs(data, number);
    record->unix_nsecs = ft_records_get_unix_nsecs(data, number);
    record->sysUpTime = ft_records_get_sysUpTime(data, number);
    record->exaddr = ft_records_get_exaddr(data, number);
    record->srcaddr = ft_records_get_srcaddr(data, number);
    record->dstaddr = ft_records_get_dstaddr(data, number);
    record->nexthop = ft_records_get_nexthop(data, number);
    record->input = ft_records_get_input(data, number);
    record->output = ft_records_get_output(data, number);
    record->dFlows = ft_records_get_dFlows(data, number);
    record->dPkts = ft_records_get_dPkts(data, number);
    record->dOctets = ft_records_get_dOctets(data, number);
    record->First = ft_records_get_First(data, number);
    record->Last = ft_records_get_Last(data, number);
    record->srcport = ft_records_get_srcport(data, number);
    record->dstport = ft_records_get_dstport(data, number);
    record->prot = ft_records_get_prot(data, number);
    record->tos = ft_records_get_tos(data, number);
    record->tcp_flags = ft_records_get_tcp_flags(data, number);
    record->engine_type = ft_records_get_engine_type(data, number);
    record->engine_id = ft_records_get_engine_id(data, number);
    record->src_mask = ft_records_get_src_mask(data, number);
    record->dst_mask = ft_records_get_dst_mask(data, number);
    record->src_as = ft_records_get_src_as(data, number);
    record->dst_as = ft_records_get_dst_as(data, number);
    record->in_encaps = ft_records_get_in_encaps(data, number);
    record->out_encaps = ft_records_get_out_encaps(data, number);
    record->peer_nexthop = ft_records_get_peer_nexthop(data, number);
    record->router_sc = ft_records_get_router_sc(data, number);
    record->src_tag = ft_records_get_src_tag(data, number);
    record->dst_tag = ft_records_get_dst_tag(data, number);
    record->extra_pkts = ft_records_get_extra_pkts(data, number);
    record->marked_tos = ft_records_get_marked_tos(data, number);
}

u_int32 *ft_records_get_unix_secs(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.unix_secs);
}

u_int32 *ft_records_get_unix_nsecs(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.unix_nsecs);
}

u_int32 *ft_records_get_sysUpTime(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.sysUpTime);
}

u_int32 *ft_records_get_exaddr(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.exaddr);
}

u_int32 *ft_records_get_srcaddr(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.srcaddr);
}

u_int32 *ft_records_get_dstaddr(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.dstaddr);
}

u_int32 *ft_records_get_nexthop(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.nexthop);
}

u_int16 *ft_records_get_input(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.input);
}

u_int16 *ft_records_get_output(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.output);
}

u_int32 *ft_records_get_dFlows(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.dFlows);
}

u_int32 *ft_records_get_dPkts(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.dPkts);
}

u_int32 *ft_records_get_dOctets(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.dOctets);
}

u_int32 *ft_records_get_First(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.First);
}

u_int32 *ft_records_get_Last(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.Last);
}

u_int16 *ft_records_get_srcport(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.srcport);
}

u_int16 *ft_records_get_dstport(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.dstport);
}

u_int8  *ft_records_get_prot(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.prot);
}

u_int8  *ft_records_get_tos(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.tos);
}

u_int8  *ft_records_get_tcp_flags(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.tcp_flags);
}

u_int8  *ft_records_get_engine_type(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.engine_type);
}

u_int8  *ft_records_get_engine_id(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.engine_id);
}

u_int8  *ft_records_get_src_mask(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.src_mask);
}

u_int8  *ft_records_get_dst_mask(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.dst_mask);
}

u_int16 *ft_records_get_src_as(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.src_as);
}

u_int16 *ft_records_get_dst_as(struct ft_data* data, int number)
{
    return (u_int16 *)(data->records[number] + data->offsets.dst_as);
}

u_int8  *ft_records_get_in_encaps(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.in_encaps);
}

u_int8  *ft_records_get_out_encaps(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.out_encaps);
}

u_int32 *ft_records_get_peer_nexthop(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.peer_nexthop);
}

u_int32 *ft_records_get_router_sc(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.router_sc);
}

u_int32 *ft_records_get_src_tag(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.src_tag);
}

u_int32 *ft_records_get_dst_tag(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.dst_tag);
}

u_int32 *ft_records_get_extra_pkts(struct ft_data* data, int number)
{
    return (u_int32 *)(data->records[number] + data->offsets.extra_pkts);
}

u_int8  *ft_records_get_marked_tos(struct ft_data* data, int number)
{
    return (u_int8  *)(data->records[number] + data->offsets.marked_tos);
}



void ft_close(struct ft_data* data)
{
    int i;

    ftio_close(&data->io);

    for (i=0; i<data->num_records; i++) {
        free(data->records[i]);
    }
    free(data->records);

    if(data->fd) {
        close(data->fd);
    }
    free(data);
}
