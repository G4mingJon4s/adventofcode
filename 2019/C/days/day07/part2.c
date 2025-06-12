#include <limits.h>
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

struct Linked {
    int value;
    struct Linked* prev;
};
typedef struct Linked Linked;

void freeLinked(Linked* l) {
    while (l != NULL) {
        Linked* n = l->prev;
        free(l);
        l = n;
    }
}

typedef int (*OpCodeFunc)(Linked** input, int* params, int* mem, int mode, int* ptr);

int _opAdd(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) + (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return INT_MIN;
}

int _opMult(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) * (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return INT_MIN;
}

int _opInput(Linked** input, int* params, int* mem, int __mode, int* ptr) {
    (void) __mode;

    mem[params[0]] = (*input)->value;
    *input = (*input)->prev;
    *ptr += 2;

    return INT_MIN;
}

int _opOutput(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    int out = (mode == 1) ? params[0] : mem[params[0]];
    *ptr += 2;

    return out;
}

int _opJmpT(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) != 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return INT_MIN;
}

int _opJmpF(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) == 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return INT_MIN;
}

int _opLT(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) < (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]])) {
        mem[params[2]] = 1;
    } else mem[params[2]] = 0;

    *ptr += 4;

    return INT_MIN;
}

int _opEQ(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) == (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]])) {
        mem[params[2]] = 1;
    } else mem[params[2]] = 0;

    *ptr += 4;

    return INT_MIN;
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

int runAmplifier(int* ram, int* ptr, int len, Linked* inputs, int* finished) {
    while (*ptr < len && ram[*ptr] != 99) {
        int rawCode = ram[*ptr];
        int opCode = rawCode % 100;

        int paramCount = numParams(opCode);
        if (paramCount == -1) {
            printf("Invalid OpCode %d (%d).\n", opCode, rawCode);
            *finished = 1;
            return INT_MIN;
        }
        rawCode /= 100;

        int values[paramCount];
        for (int i = 0; i < paramCount; i++) {
            values[i] = ram[*ptr + i + 1];
        }

        int ret = opCodeFuncs[opCode](&inputs, values, ram, rawCode, ptr);
        if (ret != INT_MIN) {
            *finished = 0;
            return ret;
        }
    }

    *finished = 1;
    return INT_MIN;
}

int testSeq(const int* mem, int len, const int* seq) {
    int* rams = malloc(5 * len * sizeof(int));
    for (int i = 0; i < 5; i++) memcpy(&rams[i * len], mem, len * sizeof(int));
    int ptrs[] = { 0, 0, 0, 0, 0 };

    Linked inputs = { 0 };
    inputs.prev = NULL;

    for (int i = 0; i < 5; i++) {
        Linked seqinp = { 0 };
        seqinp.value = seq[i];
        seqinp.prev = &inputs;

        int f;
        int o = runAmplifier(&rams[i * len], &ptrs[i], len, &seqinp, &f);
        inputs.value = o;
    }

    int out = inputs.value;
    int i = 0;
    while (i != 5) {

        int finished = 0;
        int o = runAmplifier(&rams[i * len], &ptrs[i], len, &inputs, &finished);
        if (finished) break;

        inputs.value = o;
        if (i == 4) out = o;

        i = (i + 1) % 5;
    }

    printf("Ret: %d.\n", out);
    free(rams);
    return out;
}

void swap(int* a, int* b) {
    int tmp = *b;
    *b = *a;
    *a = tmp;
}

void permutate(const int* mem, int len, int* out, int* arr, int start, int end) {
    if (start == end) {
        printf("Testing permutation [%d %d %d %d %d]\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
        int ret = testSeq(mem, len, arr);
        if (*out < ret) *out = ret;
        return;
    }

    for (int i = start; i <= end; i++) {
        swap(&arr[start], &arr[i]);
        permutate(mem, len, out, arr, start + 1, end);
        swap(&arr[start], &arr[i]);
    }
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

    printf("Setup complete.\n");

    int best = 0;
    int arr[] = { 5, 6, 7, 8, 9 };
    permutate(mem, segments->len, &best, arr, 0, 4);

    printf("Maximum output: %d\n", best);

    free(segments);
    free(mem);
}
