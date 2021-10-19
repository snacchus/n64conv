#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>

constexpr int16_t flt_to_s10_5(float f)
{
    return int16_t(f * float(1 << 5));
}

constexpr uint8_t normalize_flt_u8(float f)
{
    return uint8_t(f * 255.f);
}

constexpr int8_t normalize_flt_i8(float f)
{
    if (f < 0.0f)
    {
        return int8_t(f * 128.f);
    }
    else
    {
        return int8_t(f * 127.f);
    }
}

#endif