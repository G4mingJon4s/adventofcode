#include <limits.h>
#include <stdlib.h>
#include "../../lib/util.h"

int W = 25;
int H = 6;

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

    int i = 0;
    int bestZeros = INT_MAX;
    int result = INT_MAX;

    while (i < data->size) {
        int zeros = 0;
        int ones = 0;
        int twos = 0;

        for (int j = 0; j < W * H; j++) {
            if (i + j >= data->size) {
                printf("Unfinished layer.\n");
                break;
            }
            char cur = data->chars[i + j];
            if (cur == '0') zeros++;
            if (cur == '1') ones++;
            if (cur == '2') twos++;
        }

        if (bestZeros > zeros) {
            bestZeros = zeros;
            result = ones * twos;
        }

        i += W * H;
    }

    printf("Best result: %d\n", result);

    free(data->chars);
    free(data);
}
