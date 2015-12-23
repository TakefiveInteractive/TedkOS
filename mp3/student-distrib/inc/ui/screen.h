#ifndef _UI_SCREEN_H_
#define _UI_SCREEN_H_

#include <stdint.h>
#include <stddef.h>

namespace ui {

constexpr int32_t ScreenWidth = 1024;
constexpr int32_t ScreenHeight = 768;

template<int32_t width, int32_t height>
constexpr size_t RGBASize = width * height * 4;

template<int32_t width, int32_t height>
constexpr size_t RGBSize = width * height * 3;

constexpr size_t CalcRGBASize(int32_t width, int32_t height) { return width * height * 4; }
constexpr size_t CalcRGBSize(int32_t width, int32_t height) { return width * height * 3; }

}

#endif
