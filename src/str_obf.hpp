#pragma once
#include <array>
#include <cstdint>
template<uint8_t KEY, size_t N>
struct ObfStr {
    std::array<char, N> data;
    constexpr ObfStr(const char (&str)[N]) {
        for (size_t i = 0; i < N; i++) data[i] = str[i] ^ KEY;
    }
    void decrypt(char* out) const {
        for (size_t i = 0; i < N; i++) out[i] = data[i] ^ KEY;
    }
};
#define OBF_KEY 0x4F
#define OBF(s) ([]() -> const char* { \
    static constexpr auto _o = ObfStr<OBF_KEY>(s); \
    static char _buf[sizeof(s)]; \
    static bool _done = false; \
    if (!_done) { _o.decrypt(_buf); _done = true; } \
    return _buf; \
}())
