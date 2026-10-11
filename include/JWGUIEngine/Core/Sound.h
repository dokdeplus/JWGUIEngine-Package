#pragma once

#include <cstddef>
#include <cstdint>
#include "Export.h"

namespace jwgui {

enum class SoundEffect {
    Notice,
    Warning,
    Error
};

class JWGUI_API Sound {
public:
    static void PlayToneAsync(unsigned int frequencyHz, unsigned int durationMs);
    static void PlayClickAsync();
    static void PlayEffectAsync(SoundEffect effect);
    static void PlaySamplesAsync(const int16_t* samples, size_t sampleCount, int sampleRate);
};

} // namespace jwgui
