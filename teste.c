#include <stdio.h>

int main() {
    int * ptr;
    int ** ptr2;

    ptr2 = &ptr;

    int aux = 10;
    ptr = &aux;

    printf("%d\n", **ptr2);
}
