#include "../../lib/util.h"

int main() {
    String* data = openDataFile();

    unsigned int min;
    unsigned int max;
    int ret = sscanf(data->chars, "%d-%d", &min, &max);
    if (ret != 2) {
        printf("Couldn't parse data.\n");
        return 1;
    }

    int valid = 0;
    unsigned int num;
    for (num = min; num <= max; num++) {
        unsigned int cur = num;
        int cons = 2;

        int usedDigit = -1;
        int repeating = 0;
        int lastDigit = cur % 10;
        while ((cur /= 10) != 0) {
            int curDigit = cur % 10;
            if (curDigit == lastDigit) {
                repeating++;
                if (repeating == 1 && usedDigit == -1) {
                    cons |= 1;
                    usedDigit = curDigit;
                }
                if (repeating > 1 && usedDigit == curDigit) {
                    cons &= 2;
                    usedDigit = -1;
                }
            } else {
                repeating = 0;
            }
            if (curDigit > lastDigit) cons &= 1;
            lastDigit = curDigit;
        }
        if (cons == 3) valid++;
    }

    printf("Valid: %d\n", valid);
}
