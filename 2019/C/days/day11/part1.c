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
    BASE,
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
    if (code == BASE) return 1;

    return -1;
}

struct Point {
    long long x;
    long long y;
};
typedef struct Point Point;

struct Paint {
    Point point;
    long long value;
    struct Paint* next;
};
typedef struct Paint Paint;

Paint* findPainted(Paint* painted, Point* p) {
    while (painted != NULL) {
        if (painted->point.x == p->x && painted->point.y == p->y) return painted;
        painted = painted->next;
    }

    return NULL;
}

void freePainted(Paint* c) {
    while (c != NULL) {
        Paint* n = c->next;
        free(c);
        c = n;
    }
}

struct Linked {
    long long value;
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

void printLinked(Linked* l) {
    printf("(");

    while (l != NULL) {
        printf("%lld", l->value);
        l = l->prev;
        if (l != NULL) printf(" <- ");
    }

    printf(")");
}

typedef struct Memory {
    long long* values;
    long len;
    long ptr;
    long base;
} Memory;

void ensureAvailable(Memory* mem, long pos) {
    if (pos < 0) {
        printf("Trying to access negative position!\n");
        return;
    }
    if (mem->len > pos) return;

    long long* newSpace = realloc(mem->values, (pos + 1) * sizeof(long long));
    mem->values = newSpace;

    for (long i = mem->len; i <= pos; i++) mem->values[i] = 0;
    mem->len = pos + 1;
}

typedef long long (*OpCodeFunc)(Memory* mem, Linked** input, long long* params, int mode);

long long* mapModes(Memory* mem, long long* params, int numParams, int modes, int writes) {
    long long* outputs = malloc(numParams * sizeof(long long));

    for (int i = 0; i < numParams; i++) {
        int mode = modes % 10;
        int write = writes % 10;
        modes /= 10;
        writes /= 10;

        long long value;

        if (write == 1) {
            if (mode == 0) {
                ensureAvailable(mem, params[i]);
                value = params[i];
            } else if (mode == 1) {
                printf("Trying to write into literal!\n");
                exit(1);
            } else {
                ensureAvailable(mem, mem->base + params[i]);
                value = mem->base + params[i];
            }
        } else {
            if (mode == 0) {
                ensureAvailable(mem, params[i]);
                value = mem->values[params[i]];
            } else if (mode == 1) {
                value = params[i];
            } else {
                ensureAvailable(mem, mem->base + params[i]);
                value = mem->values[mem->base + params[i]];
            }
        }

        outputs[i] = value;
    }

    return outputs;
}

long long _opAdd(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 3, mode, 100);
    mem->values[values[2]] = values[0] + values[1];
    mem->ptr += 4;

    free(values);
    return LLONG_MIN;
}

long long _opMult(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 3, mode, 100);
    mem->values[values[2]] = values[0] * values[1];
    mem->ptr += 4;

    free(values);
    return LLONG_MIN;
}

long long _opInput(Memory* mem, Linked** input, long long* params, int mode) {
    long long* values = mapModes(mem, params, 1, mode, 1);
    mem->values[values[0]] = (*input)->value;
    *input = (*input)->prev;
    mem->ptr += 2;

    free(values);
    return LLONG_MIN;
}

long long _opOutput(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 1, mode, 0);
    long long out = values[0];
    mem->ptr += 2;

    free(values);
    return out;
}

long long _opJmpT(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 2, mode, 0);

    if (values[0] == 1) mem->ptr = values[1];
    else mem->ptr += 3;

    free(values);
    return LLONG_MIN;
}

long long _opJmpF(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 2, mode, 0);

    if (values[0] == 0) mem->ptr = values[1];
    else mem->ptr += 3;

    free(values);
    return LLONG_MIN;
}

long long _opLT(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 3, mode, 100);
    if (values[0] < values[1]) mem->values[values[2]] = 1;
    else mem->values[values[2]] = 0;
    mem->ptr += 4;

    free(values);
    return LLONG_MIN;
}

long long _opEQ(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 3, mode, 100);
    if (values[0] == values[1]) mem->values[values[2]] = 1;
    else mem->values[values[2]] = 0;
    mem->ptr += 4;

    free(values);
    return LLONG_MIN;
}

long long _opBase(Memory* mem, Linked** __input, long long* params, int mode) {
    (void) __input;

    long long* values = mapModes(mem, params, 1, mode, 0);
    mem->base += values[0];
    mem->ptr += 2;

    free(values);
    return LLONG_MIN;
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
    _opBase,
};

long long runIntcode(Memory* mem, Linked** inputs, int* finished) {
    while (mem->ptr < mem->len && mem->values[mem->ptr] != 99) {
        int rawCode = mem->values[mem->ptr];
        int opCode = rawCode % 100;

        int paramCount = numParams(opCode);
        if (paramCount == -1) {
            printf("Invalid OpCode %d (%d).\n", opCode, rawCode);
            *finished = 1;
            return LLONG_MIN;
        }
        rawCode /= 100;

        long long values[paramCount];
        for (int i = 0; i < paramCount; i++) {
            values[i] = mem->values[mem->ptr + i + 1];
        }

        long ret = opCodeFuncs[opCode](mem, inputs, values, rawCode);
        if (ret != LLONG_MIN) {
            *finished = 0;
            return ret;
        }
    }

    *finished = 1;
    return LLONG_MIN;
}

Linked* runUntilFinished(Memory* mem, Linked* inputs) {
    Linked* out = NULL;

    int finished = 0;
    while (!finished) {
        long ret = runIntcode(mem, &inputs, &finished);

        if (ret != LLONG_MIN) {
            Linked* nextOut = malloc(sizeof(Linked));
            nextOut->value = ret;
            nextOut->prev = out;
            out = nextOut;
        }
    }

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
    long len = segments->len;
    free(data->chars);
    free(data);

    long long* values = malloc(segments->len * sizeof(long long));
    int i;
    int correct = 1;
    for (i = 0; i < segments->len; i++) {
        int ret = sscanf(segments->arr[i].chars, "%lld", &values[i]);
        if (ret != 1) correct = 0;

        free(segments->arr[i].chars);
    }

    free(segments);

    if (!correct) {
        printf("Couldn't parse input.\n");
        free(values);
        return 1;
    }

    Memory mem;
    mem.len = len;
    mem.values = values;
    mem.ptr = 0;
    mem.base = 0;

    Paint* painted = NULL;
    Point robot = { 0, 0 };
    int facing = 0;

    while(1) {
        int finished = 0;
        Paint* cur = findPainted(painted, &robot);
        Linked input = { cur == NULL ? 0 : cur->value, NULL };
        Linked* inputs = &input;

        long long newPaint = runIntcode(&mem, &inputs, &finished);
        long long rotate = runIntcode(&mem, &inputs, &finished);

        if (cur != NULL) cur->value = newPaint;
        else {
            Paint* new = malloc(sizeof(Paint));
            new->value = newPaint;
            new->point.x = robot.x;
            new->point.y = robot.y;
            new->next = painted;
            painted = new;
        }

        facing += 2 * rotate - 1;
        if (facing < 0) facing = 3;
        if (facing > 3) facing = 0;

        switch (facing) {
            case 0: { robot.y++; break; }
            case 1: { robot.x++; break; }
            case 2: { robot.y--; break; }
            case 3: { robot.x--; break; }
        }

        if (finished) break;
    }

    int numPainted = 0;
    Paint* cur = painted;
    while (cur != NULL) {
        numPainted++;
        cur = cur->next;
    }

    printf("Spots painted: %d\n", numPainted);

    freePainted(painted);
    free(mem.values);
}
