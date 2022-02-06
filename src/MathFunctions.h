
//std
#include <random>
#include <time.h>

int randomRange(int min, int max) //range : [min, max]
{
    static bool first = true;
    if (first)
    {
        srand((unsigned int)time(NULL)); //seeding for the first time only!
        first = false;
    }
    return min + rand() % ((max + 1) - min);
}