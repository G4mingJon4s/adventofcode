#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lib/util.h"

int testPair(const int* init, int len, int noun, int verb) {
    int* mem = malloc(len * sizeof(int));
    memcpy(mem, init, len * sizeof(int));

    mem[1] = noun;
    mem[2] = verb;

    int ptr = 0;
    while (ptr < len && mem[ptr] != 99) {
        if (mem[ptr] != 1 && mem[ptr] != 2) {
            free(mem);
            return -1;
        }

        int a = mem[ptr + 1];
        int b = mem[ptr + 2];
        int ret = mem[ptr + 3];
        int aVal = mem[a];
        int bVal = mem[b];

        int res = mem[ptr] == 1 ? aVal + bVal : aVal * bVal;
        mem[ret] = res;
        ptr += 4;
    }

    int ret = mem[0];
    free(mem);

    return ret;
}

int main() {
    String* data = openDataFile();
    if (data == NULL) {
        printf("Couldn't open file.\n");
        return 1;
    }

    if (data->chars[data->size - 1] == '\n') {
        data->size--;
        data->chars[data->size] = '\0';
    }

    Lines* segments = splitLines(data, ',');

    int* mem = malloc(segments->len * sizeof(int));
    int i;
    for (i = 0; i < segments->len; i++) {
        int ret = sscanf(segments->arr[i].chars, "%d", &mem[i]);
        if (ret != 1) {
            printf("Couldn't parse input.\n");
            return 1;
        }
    }

    int target = 19690720;
    int noun, verb;
    for (noun = 0; noun < 100; noun++) {
        for (verb = 0; verb < 100; verb++) {
            int ret = testPair(mem, segments->len, noun, verb);
            if (ret == target) {
                printf("The answer is %d%d.\n", noun, verb);
                return 0;
            }
        }
    }

    printf("No solution found.\n");
}
