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

#ifndef f_engine_ipfix_h
#define f_engine_ipfix_h

#include <fixbuf/public.h>
#include <glib.h>

#include "ipfix-constants.h"
#include "pipeline.h"

typedef struct ipfix_ie_s {
  const char        *name;
  size_t             offset;
} ipfix_ie_t;

typedef struct ipfix_templ_s {
  ipfix_ie_t   *arr;
  size_t        len;
  size_t        next_offset;
} ipfix_templ_t;

int
ipfix_register_ie(const char * const ie_name,
                  ipfix_templ_t* templ);

uint64_t
ipfix_get_type_enum(const char * const ie_name);

uint64_t
ipfix_get_offset(const char * const         ie_name,
                 const ipfix_templ_t* const templ);

struct fbInfoElementSpec_st*
ipfix_get_fixbuf_template(ipfix_templ_t* templ);

#endif
