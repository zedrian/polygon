#include <iostream>


using std::cout;
using std::endl;


float stupidMin(float a,
                float b)
{
    return a < b ? a : b;
}

float stupidMax(float a,
                float b)
{
    return a > b ? a : b;
}


int main()
{
    float a = 2.0f;
    float b = 3.0f;

    cout << "a = 2.0" << endl;
    cout << "b = 3.0" << endl;
    cout << "stupidMin(a, b) = " << stupidMin(a, b) << endl;
    cout << "stupidMax(a, b) = " << stupidMax(a, b) << endl;
}
