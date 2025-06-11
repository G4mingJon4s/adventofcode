#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lib/util.h"

enum OpCode {
    ADD = 1,
    MULT,
    INPUT,
    OUTPUT,
    JMPT,
    JMPF,
    LT,
    EQ,
};

int numParams(enum OpCode code) {
    if (code == ADD) return 3;
    if (code == MULT) return 3;
    if (code == INPUT) return 1;
    if (code == OUTPUT) return 1;
    if (code == JMPT) return 2;
    if (code == JMPF) return 2;
    if (code == LT) return 3;
    if (code == EQ) return 3;

    return -1;
}

struct Return {
    int value;
    struct Return* prev;
};
typedef struct Return Return;

typedef Return* (*OpCodeFunc)(int input, int* params, int* mem, int mode, int* ptr);

Return* _opAdd(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) + (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return NULL;
}

Return* _opMult(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) * (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return NULL;
}

Return* _opInput(int input, int* params, int* mem, int __mode, int* ptr) {
    (void) __mode;

    mem[params[0]] = input;
    *ptr += 2;

    return NULL;
}

Return* _opOutput(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    Return* out = malloc(sizeof(Return));
    out->prev = NULL;
    out->value = mode == 1 ? params[0] : mem[params[0]];
    *ptr += 2;

    return out;
}

Return* _opJmpT(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) != 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return NULL;
}

Return* _opJmpF(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) == 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return NULL;
}

Return* _opLT(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) < (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]])) {
        mem[params[2]] = 1;
    } else mem[params[2]] = 0;

    *ptr += 4;

    return NULL;
}

Return* _opEQ(int __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) == (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]])) {
        mem[params[2]] = 1;
    } else mem[params[2]] = 0;

    *ptr += 4;

    return NULL;
}

OpCodeFunc opCodeFuncs[] = {
    NULL,
    _opAdd,
    _opMult,
    _opInput,
    _opOutput,
    _opJmpT,
    _opJmpF,
    _opLT,
    _opEQ,
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

        Return* ret = opCodeFuncs[opCode](5, values, mem, rawCode, &ptr);
        if (ret != NULL) {
            ret->prev = out;
            out = ret;
        }
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

        free(segments->arr[i].chars);
    }

    Return* ret = runTest(mem, segments->len);
    free(segments);
    free(mem);

    printf("Code: %d\n", ret->value);
    while (ret != NULL) {
        Return* prev = ret->prev;
        free(ret);
        ret = prev;
    }
}
