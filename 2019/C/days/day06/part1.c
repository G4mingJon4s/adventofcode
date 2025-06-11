#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../lib/util.h"

int hashName(const char name[4]) {
    return 100 * name[0] + 10 * name[1] + name[2];
}

typedef struct Link {
    char name[4];
    char orbits[4];

    struct Link* next;
} Link;

const char NO_ORBIT[] = "   ";

typedef struct {
    Link** buckets;
    int size;
} Map;

typedef struct Orbiters {
    char name[4];

    struct Orbiters* next;
} Orbiters;

void freeOrbiters(Orbiters* orbs) {
    while (orbs != NULL) {
        Orbiters* next = orbs->next;
        free(orbs);
        orbs = next;
    }
}

void addOrbiter(Orbiters** orbs, const char* name) {
    Orbiters* entry = calloc(1, sizeof(Orbiters));
    memcpy(entry->name, name, 4);

    if (*orbs != NULL) entry->next = *orbs;
    *orbs = entry;
}

Link* mapGet(Map* map, char* name) {
    int hash = hashName(name);
    Link* cur = map->buckets[hash % map->size];

    while (cur != NULL && strcmp(cur->name, name) != 0) cur = cur->next;

    return cur;
}

Link* mapAdd(Map* map, const char* name, const char* orbits) {
    int hash = hashName(name);

    Link* obj = malloc(sizeof(Link));
    memcpy(obj->name, name, 4);
    memcpy(obj->orbits, orbits, 4);

    obj->next = map->buckets[hash % map->size];
    map->buckets[hash % map->size] = obj;

    return obj;
}

Map* createMap(int size) {
    Map* map = malloc(size * sizeof(Map));
    map->size = size;
    map->buckets = calloc(size, sizeof(Link*));

    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }

    return map;
}

void freeMap(Map* map) {
    for (int i = 0; i < map->size; i++) {
        Link* cur = map->buckets[i];
        while (cur != NULL) {
            Link* next = cur->next;
            free(cur);
            cur = next;
        }
    }

    free(map->buckets);
    free(map);
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

    Map* map = createMap(lines->len);
    if (map == NULL) {
        printf("Couldn't create map.\n");
        return 1;
    }

    Orbiters* orbs = NULL;

    char star[4];
    char orbiter[4];
    for (int i = 0; i < lines->len; i++) {
        if (lines->arr[i].size < 7) {
            printf("Couldn't parse data (length).\n");
            return 1;
        }

        int ret = sscanf(lines->arr[i].chars, "%3s)%3s", star, orbiter);
        free(lines->arr[i].chars);
        if (ret != 2) {
            printf("Couldn't parse data (content).\n");
            freeMap(map);
            freeOrbiters(orbs);
            return 1;
        }

        if (mapGet(map, star) == NULL) mapAdd(map, star, NO_ORBIT);
        Link* orbi = mapGet(map, orbiter);
        if (orbi != NULL) {
            if (strcmp(orbi->orbits, NO_ORBIT) != 0) {
                printf("Couldn't add orbiter, already orbiting something.\n");
                printf("%s)%s (%s)\n", mapGet(map, orbiter)->orbits, mapGet(map, orbiter)->name, star);
                freeMap(map);
                freeOrbiters(orbs);
                return 1;
            }

            memcpy(orbi->orbits, star, 4);
        } else mapAdd(map, orbiter, star);
        addOrbiter(&orbs, orbiter);
    }

    free(lines->arr);
    free(lines);

    int sum = 0;
    Orbiters* cur = orbs;
    while (cur != NULL) {
        int depth = 0;
        Link* entry = mapGet(map, cur->name);
        while (entry != NULL && strcmp(entry->orbits, NO_ORBIT) != 0) {
            depth++;
            entry = mapGet(map, entry->orbits);
        }
        sum += depth;
        cur = cur->next;
    }

    printf("Sum: %d\n", sum);
    freeMap(map);
    freeOrbiters(orbs);
}
