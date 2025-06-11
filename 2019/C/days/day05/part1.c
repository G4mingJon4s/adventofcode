#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lib/util.h"

enum OpCode {
    ADD = 1,
    MULT,
    INPUT,
    OUTPUT,
};

int numParams(enum OpCode code) {
    if (code == ADD) return 3;
    if (code == MULT) return 3;
    if (code == INPUT) return 1;
    if (code == OUTPUT) return 1;

    return -1;
}

struct Return {
    int value;
    struct Return* prev;
};
typedef struct Return Return;

typedef Return* (*OpCodeFunc)(int input, int* params, int* mem, int mode);

Return* _opAdd(int __input, int* params, int* mem, int mode) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) + (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    return NULL;
}

Return* _opMult(int __input, int* params, int* mem, int mode) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) * (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    return NULL;
}

Return* _opInput(int input, int* params, int* mem, int __mode) {
    (void) __mode;

    mem[params[0]] = input;
    return NULL;
}

Return* _opOutput(int __input, int* params, int* mem, int mode) {
    (void) __input;

    Return* out = malloc(sizeof(Return));
    out->prev = NULL;
    out->value = mode == 1 ? params[0] : mem[params[0]];
    return out;
}

OpCodeFunc opCodeFuncs[] = {
    NULL,
    _opAdd,
    _opMult,
    _opInput,
    _opOutput,
};

Return* runTest(const int* ram, int len) {
    int* mem = malloc(len * sizeof(int));
    memcpy(mem, ram, len * sizeof(int));

    Return* out = NULL;

    int ptr = 0;
    while (ptr < len && mem[ptr] != 99) {
        int rawCode = mem[ptr];
        int opCode = rawCode % 100;

        int paramCount = numParams(opCode);
        if (paramCount == -1) {
            printf("Invalid OpCode %d (%d).\n", opCode, rawCode);
            free(mem);
            return NULL;
        }
        rawCode /= 100;

        int values[paramCount];
        for (int i = 0; i < paramCount; i++) {
            values[i] = mem[ptr + i + 1];
        }

        Return* ret = opCodeFuncs[opCode](1, values, mem, rawCode);
        if (ret != NULL) {
            ret->prev = out;
            out = ret;
        }

        ptr += paramCount + 1;
    }

    free(mem);
    return out;
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
    free(data->chars);
    free(data);

    int* mem = malloc(segments->len * sizeof(int));
    int i;
    for (i = 0; i < segments->len; i++) {
        int ret = sscanf(segments->arr[i].chars, "%d", &mem[i]);
        if (ret != 1) {
            printf("Couldn't parse input.\n");
            return 1;
        }
    }

    Return* ret = runTest(mem, segments->len);
    int numErrors = 0;

    for (Return* cur = ret->prev; cur != NULL; cur = cur->prev) {
        if (cur->value != 0) numErrors++;
    }

    printf("Encountered errors: %d\n", numErrors);
    printf("Code: %d\n", ret->value);
}
