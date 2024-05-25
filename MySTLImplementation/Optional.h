#pragma once

#include <utility>
#include <cstring>
#include <cassert>

struct OptionalTag
{
};

constexpr OptionalTag NullOpt;

template <typename OptionalType>
class TOptional
{
public:
    using SelfClass = TOptional<OptionalType>;

    TOptional() :
        TOptional(NullOpt)
    {
    }

    TOptional(OptionalTag) :
        valid(false)
    {
    }

    TOptional& operator=(OptionalTag)
    {
        valid = false;
        return *this;
    }

    TOptional(const OptionalType& optionalValue) :
        valid{false}
    {
        Emplace(optionalValue);
    }

    TOptional& operator=(const OptionalType& optionalValue)
    {
        Emplace(optionalValue);
        return *this;
    }

    TOptional(OptionalType&& optionalValue) :
        valid{false},
        storage{}
    {
        Emplace(std::move(optionalValue));
    }

    TOptional& operator=(OptionalType&& optionalValue)
    {
        Emplace(std::move(optionalValue));
        return *this;
    }

    TOptional(const SelfClass& optional) :
        TOptional(NullOpt)
    {
        Emplace(optional.GetValue());
    }

    TOptional& operator=(const SelfClass& optional)
    {
        Emplace(optional.GetValue());
        return *this;
    }

    TOptional(SelfClass&& optional) noexcept :
        TOptional(NullOpt)
    {
        Reset();

        if (optional.IsSet())
        {
            OptionalType value{std::move(optional.GetValue())};
            Emplace(std::move(value));
        }
    }

    TOptional& operator=(SelfClass&& optional) noexcept
    {
        Reset();

        if (optional.IsSet())
        {
            OptionalType value{std::move(optional.GetValue())};
            Emplace(std::move(value));
        }

        return *this;
    }

    ~TOptional() noexcept
    {
        Reset();
    }

    template <typename ...Args>
    void Emplace(Args&& ...args)
    {
        Reset();

        std::memset(storage, 0, sizeof(storage));
        new (&storage[0]) OptionalType(std::forward<Args>(args)...);
        valid = true;
    }

    const OptionalType& GetValue() const
    {
        assert(valid && "TOptional is not set");
        return *(const OptionalType*)&storage[0];
    }

    OptionalType& GetValue()
    {
        assert(valid && "TOptional is not set");
        return *(OptionalType*)&storage[0];
    }

    const OptionalType& operator*() const
    {
        return GetValue();
    }

    OptionalType& operator*()
    {
        return GetValue();
    }

    const OptionalType* operator->() const
    {
        return &GetValue();
    }

    OptionalType* operator->()
    {
        return &GetValue();
    }

    bool IsSet() const
    {
        return valid;
    }

    operator bool() const
    {
        return valid;
    }

    bool operator!() const
    {
        return !valid;
    }

    void Reset()
    {
        if (valid)
        {
            DestroyHoldElement();
        }
    }

private:
    uint8_t storage[sizeof(OptionalType)];
    bool valid;

    void DestroyHoldElement()
    {
        OptionalType* type = (OptionalType*)storage;
        type->~OptionalType();
        valid = false;
    }
};