#include <iostream>

using std::cerr;
using std::endl;

extern "C" {

void assertFailed(const char* source, const char* file, uint16_t line)
{
    cerr << "[" << source << "] Assert failed: " << file << ":" << line << endl;
}

void assertEFM(const char* file, int line)
{
    assertFailed("EFM", file, line);
}
}
