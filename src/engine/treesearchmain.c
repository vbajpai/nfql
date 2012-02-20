#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "ftreader.h"
#include "flowy.h"

struct grouper_rule2 {
    size_t field_offset1;
    size_t field_offset2;
    uint64_t delta;
    struct bsearch_handle *handle;
    void (*func)(
            struct group group;
            struct grouper_rule2 *rules;
            );
};

void grouper_eq_uint32_t(struct group group, struct grouper_rule2 *rules)
{
    // TODO: first member only right for absolute deltas
    uint32_t compar = *(uint32_t *)(group.members[0]+rules[0].field_offset1);
    uint32_t val;

    char **found_records = tree_find_uint32_t(rules[0].handle, group.members[0], rules[0].field_offset2);
    for (;;found_records++) {
        if (*found_records == NULL) {
            continue;
        }
        val = *(uint32_t *)(*found_records+rules[0].field_offset2);

        if (val != compar) {
            break;
        }

        if ((rules+1)->func != NULL) {
            (rules+1)->func(group, rules+1);
            continue;
        }

        group.num_members++;
        group.members = realloc
    }
}

void grouper_eq_uint16_t(struct group group, struct grouper_rule2 *rules)
{
}

int main() {
    struct ft_data *data;
    char **records;
    int num_records;
    struct group *groups;
    int num_groups;
    int i;

    data = ft_open(STDIN_FILENO);
    records = data->records;
    num_records = data->num_records;

    struct grouper_rule2 rules[5] = {
        { data->offsets.srcaddr, data->offsets.srcaddr, 0, tree_create_uint32_t(records, num_records, data->offsets.srcaddr), grouper_eq_uint32_t },
        { data->offsets.dstaddr, data->offsets.dstaddr, 0, tree_create_uint32_t(records, num_records, data->offsets.dstaddr), grouper_eq_uint32_t },
        { data->offsets.srcport, data->offsets.srcport, 0, tree_create_uint16_t(records, num_records, data->offsets.srcport), grouper_eq_uint16_t },
        { data->offsets.dstport, data->offsets.dstport, 0, tree_create_uint16_t(records, num_records, data->offsets.dstport), grouper_eq_uint16_t },
        { 0, 0, 0, NULL }
    };

    char **filtered_records;

    filtered_records = (char **)malloc(sizeof(char *)*num_records);
    if (filtered_records == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_records; i++) {
        filtered_records[i] = records[i];
    }

    num_groups = 1;
    for (i = 0; i < num_records; i++) {
        if (filtered_records[i] == NULL) {
            continue;
        }

        groups = (struct group *)realloc(sizeof(struct group)*num_groups);
        if (groups == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        groups[num_groups-1].num_members = 1;
        groups[num_groups-1].members = (char **)malloc(sizeof(char *)*groups[num_groups-1].num_members);
        groups[num_groups-1].members[0] = filtered_records[i];

        rules[0].func(groups[num_groups-1], rules);
    }

    // dont forget to merge modules to real groups

    return EXIT_SUCCESS;
}
