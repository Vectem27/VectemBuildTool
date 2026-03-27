#include <iostream>

#include "test.h"
#include "EngineTest.hpp"

int main()
{
    std::cout << "Working : " << TestFunc() << std::endl;

    PrintSame ps;

    std::cout << "Working : " << ps.GiveInline("Test") << std::endl;

    ps.Print("Working : Test 2");

    return 0;
}