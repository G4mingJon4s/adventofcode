#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

String* __openDataFile(const char* filename) {
    char dataPath[] = "data/dayXX.txt";

    int ret = sscanf(filename, "days/day%c%c/part%*c.c", dataPath + 8, dataPath + 9);
    if (ret != 2) return NULL;

    FILE* fileHandle = fopen(dataPath, "r");
    if (fileHandle == NULL) return NULL;

    fseek(fileHandle, 0, SEEK_END);
    long fsize = ftell(fileHandle);
    char* contents = malloc(fsize + 1);

    fseek(fileHandle, 0, SEEK_SET);
    fread(contents, fsize, 1, fileHandle);
    contents[fsize] = '\0';

    fclose(fileHandle);

    String* out = malloc(sizeof(String));
    out->size = fsize;
    out->chars = contents;
    return out;
}

Lines* splitLines(const String* contents, char del) {
    int numLines = 1;
    long idx;
    for (idx = 0; idx < contents->size; idx++) {
        if (contents->chars[idx] == del) numLines++;
    }
    if (contents->chars[contents->size - 1] == del) numLines--;

    String* arr = malloc(numLines * sizeof(String));
    int currentLine = 0;
    long start = 0;
    for (idx = 0; idx < contents->size; idx++) {
        if (contents->chars[idx] != del) continue;

        long len = idx - start + 1;
        char* line = malloc(len);
        if (len > 1) memcpy(line, contents->chars + start, len - 1);
        line[len - 1] = '\0';

        arr[currentLine].size = len - 1;
        arr[currentLine++].chars = line;

        start = idx + 1;
    }
    long len = idx - start + 1;
    if (len > 1 && currentLine < numLines) {
        char* line = malloc(len);
        memcpy(line, contents->chars + start, len - 1);
        line[len - 1] = '\0';

        arr[currentLine].size = len - 1;
        arr[currentLine].chars = line;
    }

    Lines* out = malloc(sizeof(Lines));
    out->arr = arr;
    out->len = numLines;

    return out;
}
