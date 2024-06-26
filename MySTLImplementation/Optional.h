#pragma once

#include <utility>
#include <cstring>
#include <cassert>
#include "InlineStorage.h"

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
        m_bValid(false)
    {
    }

    TOptional& operator=(OptionalTag)
    {
        m_bValid = false;
        return *this;
    }

    TOptional(const OptionalType& optionalValue) :
        m_bValid{false}
    {
        Emplace(optionalValue);
    }

    TOptional& operator=(const OptionalType& optionalValue)
    {
        Emplace(optionalValue);
        return *this;
    }

    TOptional(OptionalType&& optionalValue) :
        m_bValid{false},
        m_Storage{}
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

        m_Storage.Replace(std::forward<Args>(args)...);
        m_bValid = true;
    }

    const OptionalType& GetValue() const
    {
        assert(m_bValid && "TOptional is not set");
        return *m_Storage.GetValue();
    }

    OptionalType& GetValue()
    {
        assert(m_bValid && "TOptional is not set");
        return *m_Storage.GetValue();
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
        return m_bValid;
    }

    operator bool() const
    {
        return m_bValid;
    }

    bool operator!() const
    {
        return !m_bValid;
    }

    void Reset()
    {
        if (m_bValid)
        {
            DestroyHoldElement();
        }
    }

private:
    TInlineStorage<OptionalType> m_Storage;
    bool m_bValid;

    void DestroyHoldElement()
    {
        m_Storage.Destroy();
        m_bValid = false;
    }
};