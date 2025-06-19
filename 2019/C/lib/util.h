#pragma once

#include <stdio.h>

typedef struct {
    char* chars;
    long size;
} String;

typedef struct {
    String* arr;
    int len;
} Lines;

// Remapping needed for __FILE__ to be the target path
String* __openDataFile(const char* filename);
#define openDataFile() __openDataFile(__FILE__)

Lines* splitLines(const String* contents, char del);
void freeLines(Lines* l);
