#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <stdint.h>

#define RGBA_FROM_HEX(hex) (u8)(hex>>16), (u8)(hex>>8 & 0xff), (u8)(hex & 0xff), (u8)(0xff)
#define PURPLE RGBA_FROM_HEX(0xff00ff)
#define BLACK RGBA_FROM_HEX(0x0)
#define WHITE RGBA_FROM_HEX(0xffffff)
#define U32_PURPLE 0xff<<24 | 0x0<<16 | 0xff<<8 | 0xff
#define U32_GREEN 0x00<<24 | 0xff<<16 | 0x00<<8 | 0x0
#define U32_BLACK 0x0
#define CHROMA_KEY 0xff00ffff
#endif
