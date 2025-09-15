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
    
    return 0;
}

