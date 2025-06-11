#include <stdio.h>
#include <stdlib.h>
#include "../../lib/util.h"

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
    mem[1] = 12;
    mem[2] = 2;

    int ptr = 0;
    while (ptr < segments->len && mem[ptr] != 99) {
        if (mem[ptr] != 1 && mem[ptr] != 2) {
            printf("Invalid operation.\n");
            return 1;
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

    printf("Result: %d\n", mem[0]);
}
