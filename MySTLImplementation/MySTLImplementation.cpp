// MySTLImplementation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "Collections.h"

enum EAttributeIndex
{
    Hp,
    HpMax,
    Mana,
    ManaMax,
    Str,
    Dex,
    NumAttributes
};

int main()
{
    TStaticArray<int32_t, NumAttributes> attributes = {0};

    attributes[Hp] = 40;
    attributes[HpMax] = 40;

    assert(attributes[Hp] == 40 && attributes[HpMax] == 40);

    TOptional<int32_t> index = NullOpt;
    if (!index.IsSet())
    {
        index = 4;

        assert(index.IsSet() && index.GetValue() == 4);
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
