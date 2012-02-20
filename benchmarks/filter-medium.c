#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "ftreader.h"
#include "flowy.h"
#include "auto_comps.h"
#include "utils.h"

char **filter(struct ft_data *data, struct filter_rule **filter_rules, int num_filter_rules, size_t *num_filtered_records)
{
    int i, j, k;
    char **filtered_records;

    *num_filtered_records = 0;
    filtered_records = (char **)malloc(sizeof(char *)**num_filtered_records);
    if (filtered_records == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < data->num_records; i++) {
        for (j = 0; j < num_filter_rules; j++) {
            for (k = 0; filter_rules[j][k].func != NULL; k++) {
                if (filter_rules[j][k].func(data->records[i], filter_rules[j][k].field_offset, filter_rules[j][k].value, filter_rules[j][k].delta))
                    goto next;
            }
            if (filter_rules[j][k].func == NULL) break;
next:       continue;
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

int main(int argc, char **argv)
{
    struct ft_data *data;
    char **filtered_records; /* temporary - are freed later */
    size_t num_filtered_records;

    data = ft_open(STDIN_FILENO);

    /*
     * custom rules
     */

    struct filter_rule filter1[3] = {
        { data->offsets.srcport, 80, 0, filter_eq_uint16_t },
        { data->offsets.dstport, 25, 0, filter_eq_uint16_t },
        { 0, 0, 0, NULL}
    };

    struct filter_rule *filter_rules[1] = {
        filter1
    };

    filtered_records = filter(data, filter_rules, 1, &num_filtered_records);

    free(filtered_records);

    //printf("%zd\n", num_filtered_records);

    exit(EXIT_SUCCESS);
}
