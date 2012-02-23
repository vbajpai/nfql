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

#define _GNU_SOURCE

#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>

#include "base_header.h"
#include "ftreader.h"
#include "flowy.h"
#include "auto_comps.h"
#include "utils.h"

struct group **grouper(char **filtered_records, size_t num_filtered_records, struct grouper_rule *group_modules, int num_group_modules, struct grouper_aggr *aggr, size_t num_group_aggr, size_t *num_groups);
struct group ***merger(struct group ***filtered_groups, size_t *num_filtered_groups, int num_threads, struct merger_rule *filter, int num_filter_rules);

// TODO: allow OR in filters
// TODO: allow grouping and merging with more than one module

/*
 * for bitwise operations the delta is the value with which the operation is
 * done as in: bitAND(flags, delta) = value
 */

/*
 * specifying two record numbers and what fields to compare
 *
 * for allen operations, the offsets are the offsets of First and Last
 * respectively and field_lengths are FIRST and LAST
 */

char **filter(struct ft_data *data, struct filter_rule *filter_rules, int num_filter_rules, size_t *num_filtered_records){
    int i, j;
    char **filtered_records;

    *num_filtered_records = 0;
    filtered_records = (char **)malloc(sizeof(char *)**num_filtered_records);
    if (filtered_records == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < data->num_records; i++) {
        for (j = 0; j < num_filter_rules; j++) {
            if (!filter_rules[j].func(data->records[i], filter_rules[j].field_offset, filter_rules[j].value, filter_rules[j].delta))
                break;
        }

        // break if a rule did not return true
        if (j < num_filter_rules)
            continue;

        (*num_filtered_records)++;
        filtered_records = (char **)realloc(filtered_records, sizeof(char *)**num_filtered_records);
        if (filtered_records == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        filtered_records[*num_filtered_records-1] = data->records[i];
    }

    return filtered_records;
}

struct group **group_filter(struct group **groups, size_t num_groups,
        struct gfilter_rule *rules, size_t num_gfilter_rules,
        size_t *num_filtered_groups){
    int i, j;
    struct group **filtered_groups;

    *num_filtered_groups = 0;
    filtered_groups = (struct group **)malloc(sizeof(struct group *)**num_filtered_groups);

    printf("foobar3\n");
    for (i = num_groups-1; i > num_groups-10; i--) {
        printf("%p\n", groups[i]);
        if (groups[i] == NULL) {
            perror("found nil");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_groups; i++) {
        for (j = 0; j < num_gfilter_rules; j++) {
            if (!rules[j].func(groups[i], rules[j].field, rules[j].value, rules[j].delta))
                break;
        }

        if (j < num_gfilter_rules) {
            free(groups[i]->members);
            free(groups[i]->aggr);
            free(groups[i]);
            groups[i] = NULL;
            continue;
        }

        (*num_filtered_groups)++;
        filtered_groups = (struct group **)realloc(filtered_groups, sizeof(struct group *)**num_filtered_groups);
        filtered_groups[*num_filtered_groups-1] = groups[i];
    }

    filtered_groups = (struct group **)realloc(filtered_groups, sizeof(struct group *)**num_filtered_groups+1);
    if (filtered_groups == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    filtered_groups[*num_filtered_groups] = groups[i];

    printf("foobar2\n");
    for (i = *num_filtered_groups-1; i > *num_filtered_groups-10; i--) {
        printf("%p\n", filtered_groups[i]);
        if (filtered_groups[i] == NULL) {
            perror("found nil");
            exit(EXIT_FAILURE);
        }
    }

    return filtered_groups;
}

static void *branch_start(void *arg){
    struct branch_info *binfo = (struct branch_info *)arg;

    char **filtered_records; /* temporary - are freed later */
    size_t num_filtered_records;
    struct group **groups; /* temporary - are freed later */
    size_t num_groups;
    struct group **filtered_groups; /* returned */
    size_t num_filtered_groups; /* stored in binfo */

    //if (binfo->branch_id == 1) return NULL;

    /*
     * FILTER
     */

    filtered_records = filter(binfo->data, binfo->filter_rules, binfo->num_filter_rules, &num_filtered_records);
    printf("\rnumber of filtered records: %zd\n", num_filtered_records);

    /*
     * GROUPER
     */

    groups = grouper(filtered_records, num_filtered_records, binfo->group_modules, binfo->num_group_modules, binfo->aggr, binfo->num_aggr, &num_groups);
    free(filtered_records);
    printf("\nnumber of groups: %zd\n", num_groups);

    int i;
    printf("foobar4\n");
    for (i = num_groups-1; i > num_groups-10; i--) {
        printf("%p\n", groups[i]);
    }

    /*
     * GROUPFILTER
     */

    filtered_groups = group_filter(groups, num_groups, binfo->gfilter_rules, binfo->num_gfilter_rules, &num_filtered_groups);
    free(groups);
    printf("\rnumber of filtered groups: %zd\n", num_filtered_groups);

    binfo->num_filtered_groups = num_filtered_groups;
    binfo->filtered_groups = filtered_groups;

    for (i = 0; i < num_filtered_groups; i++) {
        if (binfo->filtered_groups[i] == NULL) {
            perror("found nil");
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv){
    struct ft_data *data;
    int inputFd;
    int num_threads;
    int i, ret;
    pthread_t *thread_ids;
    pthread_attr_t *thread_attrs;
    struct branch_info *binfos;
    struct group ***filtered_groups;
    size_t *num_filtered_groups;
    struct group ***group_tuples;

    num_threads = 2;
  
    if (argc != 2 || strcmp(argv[1], "--help") == 0) 
      usageErr("%s $TRACE\n", argv[0], argv[0]);
  
    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1)
      errExit("open");
  
    data = ft_open(inputFd);
  
    binfos = (struct branch_info *)calloc(num_threads, sizeof(struct branch_info));
    if (binfos == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /*
     * custom rules
     */

    struct filter_rule filter_rules_branch1[1] = {
        { data->offsets.dstport, 80, 0, filter_eq_uint16_t },
    };

    struct grouper_rule group_module_branch1[2] = {
        { data->offsets.srcaddr, data->offsets.srcaddr, 0, RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
        { data->offsets.dstaddr, data->offsets.dstaddr, 0, RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
//        { data->offsets.Last, data->offsets.First, 1, grouper_lt_uint32_t_rel }
    };

    struct grouper_aggr group_aggr_branch1[4] = {
        { 0, data->offsets.srcaddr, aggr_static_uint32_t },
        { 0, data->offsets.dstaddr, aggr_static_uint32_t },
        { 0, data->offsets.dOctets, aggr_sum_uint32_t },
        { 0, data->offsets.tcp_flags, aggr_or_uint16_t }
    };

    struct gfilter_rule gfilter_branch1[0] = {
    };

    binfos[0].branch_id = 0;
    binfos[0].data = data;
    binfos[0].filter_rules = filter_rules_branch1;
    binfos[0].num_filter_rules = 0;
    binfos[0].group_modules = group_module_branch1;
    binfos[0].num_group_modules = 2;
    binfos[0].aggr = group_aggr_branch1;
    binfos[0].num_aggr = 4;
    binfos[0].gfilter_rules = gfilter_branch1;
    binfos[0].num_gfilter_rules = 0;

    struct filter_rule filter_rules_branch2[1] = {
        { data->offsets.srcport, 80, 0, filter_eq_uint16_t },
    };

    struct grouper_rule group_module_branch2[2] = {
        { data->offsets.srcaddr, data->offsets.srcaddr, 0, RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
        { data->offsets.dstaddr, data->offsets.dstaddr, 0, RULE_EQ | RULE_NO | RULE_S2_32 | RULE_S1_32, NULL },
//        { data->offsets.Last, data->offsets.First, 1, grouper_lt_uint32_t_rel },
    };

    struct grouper_aggr group_aggr_branch2[4] = {
        { 0, data->offsets.srcaddr, aggr_static_uint32_t },
        { 0, data->offsets.dstaddr, aggr_static_uint32_t },
        { 0, data->offsets.dOctets, aggr_sum_uint32_t },
        { 0, data->offsets.tcp_flags, aggr_or_uint16_t }
    };

    struct gfilter_rule gfilter_branch2[0] = {
    };

    binfos[1].branch_id = 1;
    binfos[1].data = data;
    binfos[1].filter_rules = filter_rules_branch2;
    binfos[1].num_filter_rules = 1;
    binfos[1].group_modules = group_module_branch2;
    binfos[1].num_group_modules = 2;
    binfos[1].aggr = group_aggr_branch2;
    binfos[1].num_aggr = 4;
    binfos[1].gfilter_rules = gfilter_branch2;
    binfos[1].num_gfilter_rules = 0;

    /*
     * fill the func attribute of grouper rules
     */
    assign_fptr(binfos, num_threads);

    /*
     * SPLITTER
     *
     * (mostly pthread stuff)
     */

    thread_ids = (pthread_t *)calloc(num_threads, sizeof(pthread_t));
    if (thread_ids == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    thread_attrs = (pthread_attr_t *)calloc(num_threads, sizeof(pthread_attr_t));
    if (thread_attrs == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    filtered_groups = (struct group ***)malloc(num_threads*sizeof(struct group **));
    if (filtered_groups == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    num_filtered_groups = (size_t *)malloc(sizeof(size_t)*num_threads);
    if (num_filtered_groups == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_threads; i++) {
        ret = pthread_attr_init(&thread_attrs[i]);
        if (ret != 0) {
            errno = ret;
            perror("pthread_attr_init");
            exit(EXIT_FAILURE);
        }

        ret = pthread_create(&thread_ids[i], &thread_attrs[i], &branch_start, (void *)(&binfos[i]));
        if (ret != 0) {
            errno = ret;
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }

        ret = pthread_attr_destroy(&thread_attrs[i]);
        if (ret != 0) {
            errno = ret;
            perror("pthread_attr_destroy");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_threads; i++) {
        ret = pthread_join(thread_ids[i], NULL);
        if (ret != 0) {
            errno = ret;
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }

        num_filtered_groups[i] = binfos[i].num_filtered_groups;
        filtered_groups[i] = binfos[i].filtered_groups;
        printf("%zd\n", num_filtered_groups[i]);
    }

    free(thread_ids);
    free(thread_attrs);
    free(binfos);

    /*
     * MERGER
     */

    struct merger_rule mfilter[2] = {
        { 0, 0, 1, 1, 0, merger_eq },
        { 0, 2, 1, 2, 0, merger_lt },
    };

    printf("foobar1\n");
    for (i = num_filtered_groups[0]-1; i > num_filtered_groups[0]-10; i--) {
        printf("%p\n", filtered_groups[0][i]);
        if (filtered_groups[0][i] == NULL) {
            perror("found nil");
            exit(EXIT_FAILURE);
        }
    }

    group_tuples = merger(filtered_groups, num_filtered_groups, num_threads, mfilter, 2);

    free(group_tuples);

    /*
     * UNGROUPER
     */

    // TODO: free group_collections at some point

    exit(EXIT_SUCCESS);
}
