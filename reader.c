#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define xstr(s) str(s)
#define str(s) #s

#define BUF_SIZE 255

int main() {
    const char* const foo = "%" xstr(BUF_SIZE) "s";
    const int size = BUF_SIZE + 1;
    char buf[size];
    fgets(buf, BUF_SIZE, stdin);
    printf("%s", buf);

    return 0;
}