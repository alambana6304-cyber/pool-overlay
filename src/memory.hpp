#pragma once
#include <cstdint>
#include <cstring>
#include <mach-o/dyld.h>
#include "math.hpp"
template<typename T>
static inline T mem_read(uintptr_t addr){
    if(!addr)return T{};
    T val{};
    __builtin_memcpy(&val,reinterpret_cast<void*>(addr),sizeof(T));
    return val;
}
static inline uint32_t read_u32(uintptr_t a){return mem_read<uint32_t>(a);}
static inline uint64_t read_u64(uintptr_t a){return mem_read<uint64_t>(a);}
static inline int32_t  read_i32(uintptr_t a){return mem_read<int32_t>(a);}
static inline double   read_f64(uintptr_t a){return mem_read<double>(a);}
static inline Vec2 read_vec2(uintptr_t a){return{read_f64(a),read_f64(a+8)};}
static inline Vec3 read_vec3(uintptr_t a){return{read_f64(a),read_f64(a+8),read_f64(a+16)};}
static uintptr_t find_game_base(){
    uint32_t count=_dyld_image_count();
    for(uint32_t i=0;i<count;i++){
        const char*name=_dyld_get_image_name(i);
        if(!name)continue;
        if(strstr(name,"/pool")||strstr(name,"8ballpool"))
            return(uintptr_t)_dyld_get_image_vmaddr_slide(i);
    }
    return 0;
}
namespace OFF{
    constexpr uintptr_t SHARED_GAME_MGR=0x34E2238;
    constexpr uintptr_t GM_TABLE=0x2AC;
    constexpr uintptr_t TABLE_BALLS=0x2F0;
    constexpr uintptr_t BALLS_COUNT=0x4;
    constexpr uintptr_t BALLS_ENTRY=0xC;
    constexpr uintptr_t BALL_SPIN=0x10;
    constexpr uintptr_t BALL_POS=0x28;
    constexpr uintptr_t BALL_VEL=0x38;
    constexpr uintptr_t BALL_CLASS=0x78;
    constexpr uintptr_t BALL_STATE=0x7C;
    constexpr uintptr_t VIS_CUE=0x2D8;
    constexpr uintptr_t VIS_GUIDE=0x27C;
    constexpr uintptr_t AIM_ANGLE=0x18;
    constexpr uintptr_t POWER=0x280;
}
