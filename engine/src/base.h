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

#define _GNU_SOURCE

#ifndef f_engine_base_h
#define f_engine_base_h

#include <sys/types.h> /* Type definitions used by many programs */
#include <stdio.h> /* Standard I/O functions */
#include <stdlib.h> /* EXIT_SUCCESS and EXIT_FAILURE constants */
#include <errno.h> /* Declares errno and defines error constants */
#include <string.h> /* Commonly used string-handling functions */
#include <stdint.h>
#include <stdbool.h>

#define FILTER
#define GROUPER
#define GROUPERAGGREGATIONS
#define GROUPFILTER
#define MERGER
#define UNGROUPER

extern bool debug;
extern bool file;
extern char* dirpath;
extern uint8_t zlevel;

/* verbose levels */
extern bool verbose_v;
extern bool verbose_vv;
extern bool verbose_vvv;

enum verbosity_levels {
  HIGH      =     3,
  MEDIUM    =     2,
  LOW       =     1
};

#define FLOWHEADER "\nStart             End               Sif   SrcIPaddress    SrcP  DIf   DstIPaddress    DstP    P Fl Pkts       Octets\n"

#endif
