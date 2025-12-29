#ifndef LUT_H
#define LUT_H

#include "constants.h"
#include <cmath>

// Lookup tables for sine and exp2 with linear interpolation
// Call init() once at startup before any audio processing
class LUT {
private:
    static bool lutInitialized;
    static float sinLUT[OSC_LUT_SIZE];
    static float exp2LUT[EXP2_LUT_SIZE];

public:
    // Initialize lookup tables (call once at startup)
    static void init() {
        if (lutInitialized) return;
        
        for (size_t i = 0; i < OSC_LUT_SIZE; ++i) {
            const float angle = 2.0f * PI * static_cast<float>(i) * INV_OSC_LUT_SIZE;
            sinLUT[i] = std::sin(angle);
        }

        for (size_t i = 0; i < EXP2_LUT_SIZE; ++i) {
            const float x = EXP2_LUT_MIN + (static_cast<float>(i) * INV_EXP2_LUT_SIZE) * EXP2_LUT_RANGE;
            exp2LUT[i] = exp2f(x);
        }

        lutInitialized = true;
    }

    // Sine lookup with linear interpolation
    // Expects phase already wrapped to [0, 1) for best performance
    static inline float sin(float phase) {
        // Single wrap (handles most cases from oscillator)
        if (phase < 0.0f) phase += 1.0f;
        else if (phase >= 1.0f) phase -= 1.0f;

        const float index = phase * OSC_LUT_SIZE_F;
        const size_t i0 = static_cast<size_t>(index);
        const float frac = index - static_cast<float>(i0);
        const size_t i1 = (i0 + 1) & (OSC_LUT_SIZE - 1);
        
        return sinLUT[i0] + frac * (sinLUT[i1] - sinLUT[i0]);
    }

    // Exp2 lookup with linear interpolation
    // Valid range: [-20, 10] (returns 2^x)
    static inline float exp2(float x) {
        // Fast path for common case
        if (x == 0.0f) return 1.0f;
        
        // Clamp to valid range (no debug output in hot path)
        if (x < EXP2_LUT_MIN) x = EXP2_LUT_MIN;
        else if (x >= EXP2_LUT_MAX) x = EXP2_LUT_MAX - 0.001f;
        
        const float normalized = (x - EXP2_LUT_MIN) * EXP2_LUT_RANGE_INV;
        const float index_f = normalized * (EXP2_LUT_SIZE_F - 1);
        const size_t i0 = static_cast<size_t>(index_f);
        const float frac = index_f - static_cast<float>(i0);

        return exp2LUT[i0] + frac * (exp2LUT[i0 + 1] - exp2LUT[i0]);
    }

    // Square wave (expects phase in [0, 1))
    static inline float square(float phase) {
        return (phase < 0.5f) ? 1.0f : -1.0f;
    }

    // Triangle wave (expects phase in [0, 1))
    static inline float triangle(float phase) {
        return 1.0f - 2.0f * fabsf(2.0f * phase - 1.0f);
    }

    // Sawtooth wave (expects phase in [0, 1))
    static inline float saw(float phase) {
        return 1.0f - 2.0f * phase;
    }
};

// Static definitions
#ifndef LUT_STATIC_DEFINED
#define LUT_STATIC_DEFINED
bool LUT::lutInitialized = false;
float LUT::sinLUT[OSC_LUT_SIZE];
float LUT::exp2LUT[EXP2_LUT_SIZE];
#endif

#endif // LUT_H