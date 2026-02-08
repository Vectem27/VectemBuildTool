#include <iostream>

extern int TestFunc();

int main()
{
    std::cout << "Working : " << TestFunc() << std::endl;

    return 0;
}