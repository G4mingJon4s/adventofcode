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

typedef Linked* (*OpCodeFunc)(Linked** input, int* params, int* mem, int mode, int* ptr);

Linked* _opAdd(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("Add %d %d %d\n", params[0], params[1], params[2]);
    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) + (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return NULL;
}

Linked* _opMult(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("Mult %d %d %d\n", params[0], params[1], params[2]);
    mem[params[2]] = ((mode % 10) == 1 ? params[0] : mem[params[0]]) * (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]]);
    *ptr += 4;

    return NULL;
}

Linked* _opInput(Linked** input, int* params, int* mem, int __mode, int* ptr) {
    (void) __mode;

    printf("Input %d (%d)\n", params[0], (*input)->value);
    mem[params[0]] = (*input)->value;
    *input = (*input)->prev;
    *ptr += 2;

    return NULL;
}

Linked* _opOutput(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("Output %d (%d)\n", params[0], mode == 1 ? params[0]: mem[params[0]]);
    Linked* out = malloc(sizeof(Linked));
    out->prev = NULL;
    out->value = mode == 1 ? params[0] : mem[params[0]];
    *ptr += 2;

    return out;
}

Linked* _opJmpT(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("JmpT %d %d\n", params[0], params[1]);
    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) != 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return NULL;
}

Linked* _opJmpF(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("JmpF %d %d\n", params[0], params[1]);
    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) == 0) {
        *ptr = ((mode / 10) % 10) == 1 ? params[1] : mem[params[1]];
    } else *ptr += 3;

    return NULL;
}

Linked* _opLT(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("LT %d %d %d\n", params[0], params[1], params[2]);
    if (((mode % 10) == 1 ? params[0] : mem[params[0]]) < (((mode / 10) % 10) == 1 ? params[1] : mem[params[1]])) {
        mem[params[2]] = 1;
    } else mem[params[2]] = 0;

    *ptr += 4;

    return NULL;
}

Linked* _opEQ(Linked** __input, int* params, int* mem, int mode, int* ptr) {
    (void) __input;

    printf("EQ %d %d %d\n", params[0], params[1], params[2]);
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

Linked* runAmplifier(int* ram, int* ptr, int len, Linked* inputs, Linked** pass) {
    Linked* out = NULL;

    int i = 0;
    while (*ptr < len && ram[*ptr] != 99) {
        printf("Ex #%d (%d).\n", ++i, *ptr);
        int rawCode = ram[*ptr];
        int opCode = rawCode % 100;

        int paramCount = numParams(opCode);
        if (paramCount == -1) {
            printf("Invalid OpCode %d (%d).\n", opCode, rawCode);
            return NULL;
        }
        rawCode /= 100;

        int values[paramCount];
        for (int i = 0; i < paramCount; i++) {
            values[i] = ram[*ptr + i + 1];
        }

        Linked* ret = opCodeFuncs[opCode](&inputs, values, ram, rawCode, ptr);
        if (ret != NULL) {
            ret->prev = out;
            out = ret;
            *pass = out;
            return NULL;
        }
    }

    return out;
}

int testSeq(const int* mem, int len, int* seq) {
    printf("Seq with len %d\n", len);
    int* rams = malloc(5 * len);
    for (int i = 0; i < 5; i++) memcpy(&rams[i * len], mem, len);
    int ptrs[] = { 0, 0, 0, 0, 0 };

    Linked inputs = { 0 };
    inputs.prev = NULL;

    Linked* out = NULL;
    int i = 0;
    int repeat = 1;
    while (out == NULL || i != 4) {
        printf("Running: %d.\n", i);

        Linked spass = { 0 };
        spass.value = INT_MIN;
        spass.prev = NULL;
        Linked* pass = &spass;

        Linked seqinp = { 0 };
        seqinp.value = seq[i];
        seqinp.prev = &inputs;

        out = runAmplifier(&rams[i * len], &ptrs[i], len, &seqinp, &pass);

        if (out == NULL) {
            if (pass->value == INT_MIN) printf("No passthrough in loop.\n");
            inputs.value = pass->value;
        } else {
            inputs.value = out->value;
            repeat = 0;
        }

        i++;
        if (i >= 5) {
            i = 0;
            if (repeat == 0) break;
        }
    }

    free(rams);
    if (out == NULL) {
        printf("Ret: No return.\n");
        return INT_MIN;
    }

    int ret = out->value;
    printf("Ret: %d.\n", ret);
    freeLinked(out);
    return ret;
}

void swap(int* a, int* b) {
    int tmp = *b;
    *b = *a;
    *a = tmp;
}

void permutate(const int* mem, int len, int* out, int* arr, int start, int end) {
    if (start == end) {
        printf("Testing permutation...\n");
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
