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

enum class EDeviceType
{
    Null,
    OpenGL,
    DirectX9,
    DirectX11,
    DirectX12,
};

struct FVideoMode
{
    int Width;
    int Height;
    TEnumAsByte<EDeviceType> DeviceType;
    bool SupportsHardwareAcceleration : 1;
};

class IAnimal
{
public:
    virtual ~IAnimal() = default;
    virtual void DoSpeak() = 0;
};

class Dog : public IAnimal
{
public:

    // Inherited via IAnimal
    void DoSpeak() override
    {
        puts("Woof");
    }

    int32_t data[100];
};

class Cat : public IAnimal
{
public:

    // Inherited via IAnimal
    void DoSpeak() override
    {
        puts("Miauu");
    }

    float data[100];
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

    TList<TOptional<int32_t>> d;

    d.EmplaceBack(40);
    d.EmplaceBack(80);
    d.EmplaceBack(NullOpt);
    d.EmplaceBack(20);
    d.EmplaceBack(70);

    d.EmplaceBack();

    TArray<FVideoMode> devices;

    devices.EmplaceBack(FVideoMode{1280, 720, EDeviceType::DirectX11, true});
    devices.EmplaceBack();
    devices.EmplaceBack();
    devices.EmplaceBack();
    devices.EmplaceBack();


    for (auto& dev : devices)
    {
        printf("%ix%i SupportsHardware=%i DeviceIndex=%i\n", dev.Width, dev.Height,
            (int)dev.SupportsHardwareAcceleration, (int)dev.DeviceType.GetByteValue());
    }

    for (TOptional<int32_t>& i : d)
    {
        if (i.IsSet())
        {
            printf("%i\n", i.GetValue());
        }
        else
        {
            printf("NullOpt\n");
        }
    }

    TSharedPtr<IAnimal> p = MakeShared<Dog>();

    TWeakPointer<IAnimal> animal = p;
    TWeakPointer<IAnimal> animal2 = animal;

    TSharedPtr<IAnimal> p2 = p;

    p2 = p;

    animal = animal2;
    p = animal.ToShared();
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
