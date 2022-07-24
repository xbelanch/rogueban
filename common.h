#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t i32;
typedef int64_t i64;

#define RGBA_FROM_HEX(hex) (u8)(hex>>16), (u8)(hex>>8 & 0xff), (u8)(hex & 0xff), (u8)(0xff)
#define PURPLE RGBA_FROM_HEX(0xff00ff)
#define U32_PURPLE 0xff<<24 | 0x0<<16 | 0xff<<8 | 0xff
#define U32_BLACK 0x0
#define RED RGBA_FROM_HEX(0xff0000)
#define GREEN RGBA_FROM_HEX(0x00ff00)
#define BLUE RGBA_FROM_HEX(0x0000ff)
#define BLACK RGBA_FROM_HEX(0x0)
#define WHITE RGBA_FROM_HEX(0xffffff)


#endif
