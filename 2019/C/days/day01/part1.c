#include <stdio.h>
#include "../../lib/util.h"

int convertToNumber(const char* line) {
    int out;
    int ret = sscanf(line, "%d", &out);
    if (ret != 1) return -1;
    return out;
}

int main() {
    String* data = openDataFile();
    if (data == NULL) {
        printf("Couldn't open file.\n");
        return 1;
    }

    Lines* lines = splitLines(data, '\n');

    int sum = 0;
    int idx;
    for (idx = 0; idx < lines->len; idx++) {
        int conv = convertToNumber(lines->arr[idx].chars);
        if (conv == -1) {
            printf("Couldn't parse line %d.\n", idx);
            return 1;
        }

        sum += (conv / 3) - 2;
    }

    printf("Answer: %d\n", sum);
}
