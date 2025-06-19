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

    printf("Setup complete.\n");

    char img[W * H];
    for (int a = 0; a < W * H; a++) img[a] = 0;
    printf("Initialized.\n");

    int i = 0;
    while (i < data->size) {
        for (int j = 0; j < W * H; j++) {
            if (i + j >= data->size) {
                printf("Unfinished layer.\n");
                break;
            }
            char cur = data->chars[i + j];
            if (cur != '2' && img[j] == 0) img[j] = cur;
        }

        i += W * H;
    }

    printf("Image:");
    for (i = 0; i < W * H; i++) {
        if (i % W == 0) printf("\n");
        printf("%c", img[i] == '0' ? ' ' : 'W');
    }
    printf("\n");

    free(data->chars);
    free(data);
}
