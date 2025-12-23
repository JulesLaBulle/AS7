#ifndef LUT_H
#define LUT_H

#include "constants.h"
#include <cmath>

class LUT {
    private:
        static bool lutInitialized;
        static float sinLUT[OSC_LUT_SIZE];
        static float exp2LUT[EXP2_LUT_SIZE];
    
    public:
        static void init() {
            if (lutInitialized) return;
            
            // Initialize sine LUT
            for (size_t i = 0; i < OSC_LUT_SIZE; ++i) {
                const float angle = 2.0f * PI * static_cast<float>(i) * INV_OSC_LUT_SIZE;
                sinLUT[i] = static_cast<float>(std::sin(angle));
            }

            // Initialize exp2 LUT
            for (size_t i = 0; i < EXP2_LUT_SIZE; ++i) {
                const float x = EXP2_LUT_MIN + (static_cast<float>(i) / EXP2_LUT_SIZE_F) * EXP2_LUT_RANGE;
                exp2LUT[i] = exp2f(x);
            }

            lutInitialized = true;
        }        

        static inline float sin(float phase) {
            if (!lutInitialized) init();
            
            // Wrap phase to [0, 1)
            if(phase < 0.0f) phase += 1.0f;
            if(phase >= 1.0f) phase -= 1.0f;

            const float index = phase * OSC_LUT_SIZE_F;
            const size_t i0 = static_cast<size_t>(index);
            const size_t i1 = (i0 + 1) & (OSC_LUT_SIZE - 1);
            const float frac = index - static_cast<float>(i0);
            
            return sinLUT[i0] + frac * (sinLUT[i1] - sinLUT[i0]);
        }

        static inline float exp2(float x) {
            if (!lutInitialized) init();

            if(x == 0.0f) return 1.0f;
            
            float normalized = (x - EXP2_LUT_MIN) * EXP2_LUT_RANGE_INV;
            
            if (normalized < 0.0f || normalized >= 1.0f) {
                std::cout << "WARNING: exp2 input out of range: " << x << std::endl;
                return exp2f(x);
            }

            float index_f = normalized * (EXP2_LUT_SIZE_F - 1);
            size_t i0 = static_cast<size_t>(index_f);
            size_t i1 = i0 + 1;

            float frac = index_f - static_cast<float>(i0);

            return exp2LUT[i0] + frac * (exp2LUT[i1] - exp2LUT[i0]);
        }

        // Waveforms for LFO & Operators
        static inline float square(float phase) {
            // Wrap phase to [0, 1)
            if(phase < 0.0f) phase += 1.0f;
            if(phase >= 1.0f) phase -= 1.0f;

            return (phase < 0.5f) ? 1.0f : -1.0f;
        }

        static inline float triangle(float phase) {
            // Wrap phase to [0, 1)
            if(phase < 0.0f) phase += 1.0f;
            if(phase >= 1.0f) phase -= 1.0f;

            return 1.0f - 2.0f * fabsf(2.0f * phase - 1.0f);
        }

        static inline float saw(float phase) {
            // Wrap phase to [0, 1)
            if(phase < 0.0f) phase += 1.0f;
            if(phase >= 1.0f) phase -= 1.0f;

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