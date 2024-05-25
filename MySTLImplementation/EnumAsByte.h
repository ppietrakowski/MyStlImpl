#pragma once

#include <cstdint>
#include <type_traits>


template <typename TEnum>
class TEnumAsByte
{
public:
    using TEnumInt = uint8_t;

    constexpr TEnumAsByte() :
        TEnumAsByte{TEnum{}}
    {
    }

    constexpr TEnumAsByte(TEnum enumValue) :
        m_EnumAsInt{static_cast<TEnumInt>(enumValue)}
    {
    }

    constexpr TEnumAsByte(TEnumInt enumAsInt) :
        m_EnumAsInt{enumAsInt}
    {
    }

    constexpr bool operator&(TEnum other) const
    {
        TEnumInt otherEnumAsInt = static_cast<TEnumInt>(other);
        return static_cast<bool>(m_EnumAsInt & otherEnumAsInt);
    }

    constexpr TEnumAsByte operator|(TEnum other) const
    {
        TEnumInt otherEnumAsInt = static_cast<TEnumInt>(other);
        return TEnumAsByte(m_EnumAsInt | otherEnumAsInt);
    }

    constexpr operator TEnum() const
    {
        return static_cast<TEnum>(m_EnumAsInt);
    }

    constexpr TEnum GetValue() const
    {
        return static_cast<TEnum>(m_EnumAsInt);
    }

    constexpr TEnumInt GetByteValue() const
    {
        return m_EnumAsInt;
    }

    constexpr bool operator==(const TEnumAsByte<TEnum>& otherEnum) const
    {
        return otherEnum.m_EnumAsInt == m_EnumAsInt;
    }

    constexpr bool operator==(TEnum otherEnum) const
    {
        TEnumInt otherEnumAsInt = static_cast<TEnumInt>(otherEnum);
        return otherEnumAsInt == m_EnumAsInt;
    }

    constexpr bool operator!=(const TEnumAsByte<TEnum>& otherEnum) const
    {
        return otherEnum.m_EnumAsInt != m_EnumAsInt;
    }

    constexpr bool operator!=(TEnum otherEnum) const
    {
        TEnumInt otherEnumAsInt = static_cast<TEnumInt>(otherEnum);
        return otherEnumAsInt != m_EnumAsInt;
    }

private:
    TEnumInt m_EnumAsInt;
};
