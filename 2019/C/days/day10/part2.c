#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include "../../lib/util.h"

typedef struct Point {
    int x;
    int y;
} Point;

typedef struct Angles {
    Point step;
    struct Angles* next;
} Angles;

void freeAngles(Angles* a) {
    while (a != NULL) {
        Angles* n = a->next;
        free(a);
        a = n;
    }
}

Angles* invertAngles(Angles* a) {
    Angles* head = NULL;

    while (a != NULL) {
        Angles* next = a->next;
        a->next = head;
        head = a;
        a = next;
    }

    return head;
}

char readPoint(char* grid, Point* dimensions, Point* point) {
    if (point->x < 0 || point->y < 0) return -1;
    if (dimensions->x <= point->x || dimensions->x <= point->y) return -1;

    int pos = dimensions->y * point->y + point->x;
    return grid[pos];
}

void setPoint(char* grid, Point* dimensions, Point* point, char value) {
    if (point->x < 0 || point->y < 0) return;
    if (dimensions->x <= point->x || dimensions->x <= point->y) return;

    int pos = dimensions->y * point->y + point->x;
    grid[pos] = value;
}

double SINC = 0.005;
double SDEV = 0.01;
Point radiansStep(double radians, Point* dimensions) {
    double dx = sin(radians);
    double dy = cos(radians);

    double max = 2 * sqrt(dimensions->x * dimensions->x + dimensions->y * dimensions->y);
    for (double i = 0; i < max; i += SINC) {
        double sx = dx * i;
        double sy = dy * i;

        double devx = round(sx) - sx;
        double devy = round(sy) - sy;
        if (round(sx) == 0 && round(sy) == 0) continue;
        if (devx > SDEV || devx < -SDEV) continue;
        if (devy > SDEV || devy < -SDEV) continue;

        Point step = { round(sx), -round(sy) };
        return step;
    }

    Point fallback = { 0, 0 };
    return fallback;
}

int countVisible(char* grid, Point* dimensions, Point* start, Angles* angles) {
    int count = 0;

    while (angles != NULL) {
        Point step = angles->step;
        int l = 1;

        while (l < INT_MAX) {
            int x = step.x * l + start->x;
            int y = step.y * l + start->y;
            if (x < 0 || y < 0) break;
            if (dimensions->x <= x || dimensions->y <= y) break;

            Point p = { x, y };
            char entry = readPoint(grid, dimensions, &p);
            if (entry == '#') {
                count++;
                break;
            }

            l++;
        }

        angles = angles->next;
    }

    return count;
}

Point findAligned(char* grid, Point* dimensions, Point* start, Point* step) {
    int i = 1;

    while (1) {
        Point cur = { start->x + step->x * i, start->y + step->y * i };
        char entry = readPoint(grid, dimensions, &cur);
        if (entry == -1) break;
        if (entry == '#') return cur;

        i++;
    }

    Point fallback = { -1, -1 };
    return fallback;
}

double AINC = 0.00001;
double PI = 3.1415926535;

int main() {
    String* data = openDataFile();
    if (data == NULL) {
        printf("Couldn't open file.\n");
        return 1;
    }

    int W = 0;
    while (W < data->size && data->chars[W] != '\n') W++;

    int H = data->size;
    if (data->chars[data->size - 1] != '\n') H++;
    H /= (W + 1);

    printf("Dimensions: %dx%d\n", W, H);
    char grid[W * H + 1];

    int i = 0;
    for (int p = 0; p < data->size; p++) {
        if (data->chars[p] == '\n') continue;
        grid[i++] = data->chars[p];
    }
    if (i != W * H) printf("Couldn't fill the buffer properly! (i=%d | size=%d)\n", i, W * H);
    grid[i] = '\0';

    free(data->chars);
    free(data);

    Point dimensions = { W, H };
    Angles* angles = NULL;

    for (double i = 0; i < 2 * PI; i += AINC) {
        Point step = radiansStep(i, &dimensions);
        if (step.x == 0 && step.y == 0) continue;

        if (angles != NULL && angles->step.x == step.x && angles->step.y == step.y) continue;
        printf("Angles: %.2f%%\r", (100 * i) / (2 * PI));

        Angles* entry = malloc(sizeof(Angles));
        entry->step.x = step.x;
        entry->step.y = step.y;
        entry->next = angles;
        angles = entry;
    }

    printf("\33[2K\rAngles: Complete!\n");

    // Remove the last one since it is already covered first
    if (angles != NULL && angles->step.x == 0 && angles->step.y == 1) {
        Angles* n = angles->next;
        free(angles);
        angles = n;
    }

    int highestVisible = 0;
    Point coords = { -1, -1 };
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Point p = { x, y };

            char entry = readPoint(grid, &dimensions, &p);
            if (entry == -1) {
                printf("Invalid coordinates! (%d | %d)\n", x, y);
                freeAngles(angles);
                return 1;
            }

            if (entry == '#') {
                int visible = countVisible(grid, &dimensions, &p, angles);
                if (visible > highestVisible) {
                    highestVisible = visible;
                    coords.x = x;
                    coords.y = y;
                }
            }
        }
    }

    printf("Station is on (%d | %d)\n", coords.x, coords.y);
    angles = invertAngles(angles);

    Angles* cur = angles;
    int destroying = 0;
    while (destroying < 200) {
        if (cur == NULL) cur = angles;

        double ratio = cur->step.y == 0 ? -1 : ((double) cur->step.x) / cur->step.y;
        double angle = ratio == -1 ? 90.0 * (cur->step.x < 0 ? -1 : 1) : (180.0 / PI) * atan(ratio);
        printf("Destroying: %.2f%% (%.2f°)\r", destroying * 0.5, angle);

        Point destroyed = findAligned(grid, &dimensions, &coords, &cur->step);

        if (destroyed.x != -1 && destroyed.y != -1) {
            destroying++;
            setPoint(grid, &dimensions, &destroyed, '.');

            if (destroying == 200) {
                printf("\33[2K\rDestroying: Complete!\n");
                printf("Answer: %d\n", 100 * destroyed.x + destroyed.y);
                break;
            }
        }

        cur = cur->next;
    }

    freeAngles(angles);
}
