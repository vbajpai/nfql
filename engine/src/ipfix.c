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

#include "ipfix.h"


int
ipfix_ie_register(char * const ie_name,
                  ipfix_templ_t* templ) {
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

ssize_t
ipfix_templ_get_ie_offset(const ipfix_templ_t* templ,
                          const char * const ie_name) {
  ipfix_ie_t* ie = ipfix_templ_get_ie(templ, ie_name);
  if (ie == NULL) return -1;

  return ie->offset;
}

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

#undef __fbtempl_add_elem
#undef _fbtempl_add_padding
#undef _fbtempl_add

  return spec;

fail:
  free(spec);
  return NULL;
}
