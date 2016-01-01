#include <iostream>

#include <ft2build.h>
#include <freetype/freetype.h>


using std::cout;
using std::endl;


int main()
{
    FT_Library library;
    auto error = FT_Init_FreeType(&library);
    if(error)
    {
        cout << "Failed to init FreeType library." << endl;
        return -1;
    }
    cout << "FreeType library initialization succeeded." << endl;
}
