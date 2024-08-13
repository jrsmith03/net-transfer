#include "util.h"
// Convert n characters of given number into a sting
void itoa(int num, char* res, int len) {
    int i = 0;
    res[len] = '\0';
    res[--len] = num % 10 + '0';
    num /= 10;
    while (len > 0) {
        res[--len] = num % 10 + '0';
        num /= 10;
    }
}

int digits(int to_count) {
    int num = 0;
    while ((to_count = to_count / 10 )!= 0) {
        num++;
    }
    if (to_count == 0) {
        return ++num;
    }
    return num;
}
