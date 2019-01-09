#include <iostream>
#include <thread>

#include "directory_scanner/scanner.h"

using namespace std;

int main()
{
    DuplicateScanner ds("/home/georgiy/programing/sem3/hw1/test/manual/small", FileComparator());
    ds.scan();
    return 0;
}