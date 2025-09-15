#include <stdio.h>
#include <math.h>

typedef struct
{
    float x;
    float y;
} point;



int add_together(int x, int y) {
    int result = x + y;
    return result;
}


int main(int argc, char const *argv[])
{
    int count = 10;
    int added = add_together(10, 18);

    point p;
    p.x = 0.1;
    p.y = 10.0;

    float length = sqrt(p.x * p.x + p.y * p.y);

    int x = 35;

    if (x > 10 && x < 100) {
        puts("x is greater than blah blah");
    } else {
        puts("x is less than blah blah");
    }

    int i = 10;
    while (i > 0) {
        puts("Loop iteration");
        i--;
    }

    for (int i = 0; i < 10; i++) {
        puts("Loop iteration");
    }

    return 0;
}

