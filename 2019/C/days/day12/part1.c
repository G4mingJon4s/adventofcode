#include <stdio.h>
#include <stdlib.h>
#include "../../lib/util.h"

typedef struct Vec3 {
    int x;
    int y;
    int z;
} Vec3;

typedef struct Moon {
    Vec3 pos;
    Vec3 vel;
} Moon;

void printMoon(Moon* m) {
    printf("P(%d | %d | %d) V(%d | %d | %d)", m->pos.x, m->pos.y, m->pos.z, m->vel.x, m->vel.y, m->vel.z);
}

void applyGravity(Moon* arr, int num) {
    for (int i = 0; i < num - 1; i++) {
        for (int j = i + 1; j < num; j++) {
            if (arr[i].pos.x > arr[j].pos.x) {
                arr[i].vel.x--;
                arr[j].vel.x++;
            } else if (arr[i].pos.x != arr[j].pos.x) {
                arr[i].vel.x++;
                arr[j].vel.x--;
            }

            if (arr[i].pos.y > arr[j].pos.y) {
                arr[i].vel.y--;
                arr[j].vel.y++;
            } else if (arr[i].pos.y != arr[j].pos.y) {
                arr[i].vel.y++;
                arr[j].vel.y--;
            }

            if (arr[i].pos.z > arr[j].pos.z) {
                arr[i].vel.z--;
                arr[j].vel.z++;
            } else if (arr[i].pos.z != arr[j].pos.z) {
                arr[i].vel.z++;
                arr[j].vel.z--;
            }
        }
    }
}

int calculateEnergy(Moon* arr, int num) {
    int out = 0;

    for (int i = 0; i < num; i++) {
        int pot = abs(arr[i].pos.x) + abs(arr[i].pos.y) + abs(arr[i].pos.z);
        int kin = abs(arr[i].vel.x) + abs(arr[i].vel.y) + abs(arr[i].vel.z);
        out += pot * kin;
    }

    return out;
}

void applyVelocity(Moon* arr, int num) {
    for (int i = 0; i < num; i++) {
        arr[i].pos.x += arr[i].vel.x;
        arr[i].pos.y += arr[i].vel.y;
        arr[i].pos.z += arr[i].vel.z;
    }
}

int main() {
    String* data = openDataFile();
    if (data == NULL) {
        printf("Couldn't open file.\n");
        return 1;
    }

    Lines* lines = splitLines(data, '\n');
    free(data->chars);
    free(data);

    int numMoons = lines->len;
    Moon moons[numMoons];
    for (int i = 0; i < numMoons; i++) {
        int x, y, z;
        int ret = sscanf(lines->arr[i].chars, "<x=%d, y=%d, z=%d>", &x, &y, &z);
        if (ret != 3) {
            printf("Couldn't parse data.\n");
            freeLines(lines);
            return 1;
        }

        moons[i].pos.x = x;
        moons[i].pos.y = y;
        moons[i].pos.z = z;

        moons[i].vel.x = 0;
        moons[i].vel.y = 0;
        moons[i].vel.z = 0;
    }

    freeLines(lines);

    int iter = 1000;
    for (int i = 0; i < iter; i++) {
        applyGravity(moons, numMoons);
        applyVelocity(moons, numMoons);

        printf("After Step %d:\n", i + 1);
        for (int j = 0; j < numMoons; j++) {
            printMoon(&moons[j]);
            printf("\n");
        }
        printf("\n");
    }

    int energy = calculateEnergy(moons, numMoons);
    printf("Energy: %d\n", energy);
}
