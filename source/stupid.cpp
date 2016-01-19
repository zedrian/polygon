#include "stupid.h"


using namespace polygon;


float math::stupidMin(float a,
                      float b)
{
    return a < b ? a : b;
}

float math::stupidMax(float a,
                      float b)
{
    return a > b ? a : b;
}