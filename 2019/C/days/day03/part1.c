#include <limits.h>
#include <stdlib.h>
#include "../../lib/util.h"

typedef struct {
    char dir;
    int dist;
} Step;

typedef struct {
    int len;
    Step* steps;
} Wire;

Wire* parseWire(const String* line) {
    Lines* parts = splitLines(line, ',');

    Wire* wire = malloc(sizeof(Wire));
    wire->len = parts->len;
    wire->steps = malloc(wire->len * sizeof(Wire));

    int i;
    for(i = 0; i < parts->len; i++) {
        char dir;
        int dist;

        int ret = sscanf(parts->arr[i].chars, "%c%d", &dir, &dist);
        if (ret != 2 || (dir != 'L' && dir != 'R' && dir != 'U' && dir != 'D')) {
            printf("Invalid movement.\n");

            for (i = 0; i < parts->len; i++) free(parts->arr[i].chars);
            free(parts->arr);
            free(parts);
            free(wire->steps);
            free(wire);

            return NULL;
        }

        wire->steps[i].dir = dir == 'L' || dir == 'R' ? 'X' : 'Y';
        wire->steps[i].dist = dir == 'L' || dir == 'D' ? -dist : dist;
    }

    for (i = 0; i < parts->len; i++) free(parts->arr[i].chars);
    free(parts->arr);
    free(parts);

    return wire;
}

int main() {
    String* data = openDataFile();

    Lines* lines = splitLines(data, '\n');
    if (lines == NULL || lines->len != 2) {
        printf("Couldn't parse data.\n");
        return 1;
    }

    Wire* wire1 = parseWire(&lines->arr[0]);
    Wire* wire2 = parseWire(&lines->arr[1]);
    if (wire1 == NULL || wire2 == NULL) {
        printf("Couldn't parse wires.\n");
        return 1;
    }

    int distBest = INT_MAX;

    int x1 = 0;
    int y1 = 0;
    int x1n;
    int y1n;
    for (int s1 = 0; s1 < wire1->len; (s1++, x1 = x1n, y1 = y1n)) {
        Step sw1 = wire1->steps[s1];
        x1n = x1;
        y1n = y1;
        if (sw1.dir == 'X') {
            x1n += sw1.dist;
        } else {
            y1n += sw1.dist;
        }

        int x2 = 0;
        int y2 = 0;
        int x2n;
        int y2n;
        for (int s2 = 0; s2 < wire2->len; (s2++, x2 = x2n, y2 = y2n)) {
            Step sw2 = wire2->steps[s2];
            x2n = x2;
            y2n = y2;

            if (sw2.dir == 'X') {
                x2n += sw2.dist;
            } else {
                y2n += sw2.dist;
            }

            int ix;
            int iy;
            if (sw1.dir == 'X') {
                ix = x2;
                iy = y1;
                if (abs(ix) + abs(iy) > distBest || (ix == 0 && iy == 0)) continue;

                if (((x1 < ix && ix < x1n) || (x1n < ix && ix < x1)) &&
                    ((y2 < iy && iy < y2n) || (y2n < iy && iy < y2))) {
                    distBest = abs(ix) + abs(iy);
                }
            } else {
                ix = x1;
                iy = y2;
                if (abs(ix) + abs(iy) > distBest || (ix == 0 && iy == 0)) continue;

                if (((x2 < ix && ix < x2n) || (x2n < ix && ix < x2)) &&
                    ((y1 < iy && iy < y1n) || (y1n < iy && iy < y1))) {
                    distBest = abs(ix) + abs(iy);
                }
            }
        }
    }

    printf("Best distance: %d\n", distBest);
}
