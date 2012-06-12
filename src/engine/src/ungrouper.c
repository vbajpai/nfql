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

#include "ungrouper.h"

#ifdef UNGROUPER
struct ungrouper_result*
ungrouper(
          size_t num_branches,
          const struct merger_result* const mresult
         ) {

  /* free'd after returning from ungrouper(...) */
  struct ungrouper_result* uresult = calloc(1, sizeof(struct ungrouper_result));
  if (uresult == NULL)
    errExit("calloc");

  if (mresult->num_group_tuples != 0) {

    /* free'd after returning from ungrouper(...) */
    struct stream** streamset = (struct stream**)
                                calloc(mresult->num_group_tuples,
                                       sizeof(struct stream*));
    if (streamset == NULL)
      errExit("calloc");
    for (int i = 0; i < mresult->num_group_tuples; i++) {

      /* free'd after returning from ungrouper(...) */
      struct stream* stream = calloc(1, sizeof(struct stream));
      if (stream == NULL)
        errExit("calloc");

      struct group** group_tuple = mresult->group_tuples[i];
      for (int j = 0; j < num_branches; j++) {

        struct group* group = group_tuple[j];

        /* free'd after returning from ungrouper(...) */
        stream->recordset = realloc(stream->recordset,
                                   (stream->num_records +
                                    group->num_members) * sizeof(char*));
        if (stream->recordset == NULL)
          errExit("realloc");

        for (int k = stream->num_records, l = 0;
             k < (stream->num_records + group->num_members);
             k++, l++)
          stream->recordset[k] = group->members[l];
        stream->num_records += group->num_members;
      }

      streamset[i] = stream;
    }
    uresult->streamset = streamset; streamset = NULL;
    uresult->num_streams = mresult->num_group_tuples;
  }
  return uresult;
}
#endif
