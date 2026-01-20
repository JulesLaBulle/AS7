#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>
#include <cstdint>

// Audio
constexpr float SAMPLE_RATE = 44100.0f;
constexpr float INV_SAMPLE_RATE = 1.0f / SAMPLE_RATE;
constexpr float TWO_PI_F = 6.28318530718f; // 2*PI for oscillator phase calculations

// Synth
constexpr uint8_t POLYPHONY = 8;
constexpr size_t NUM_OPERATORS = 6;
constexpr float MODULATION_SCALING = 12.5f;

// LUT
constexpr size_t OSC_LUT_SIZE = 4096;
constexpr float OSC_LUT_SIZE_F = static_cast<float>(OSC_LUT_SIZE);
constexpr float INV_OSC_LUT_SIZE = 1.0f / OSC_LUT_SIZE_F;

constexpr size_t EXP2_LUT_SIZE = 4096; 
constexpr float EXP2_LUT_SIZE_F = static_cast<float>(EXP2_LUT_SIZE);
constexpr float INV_EXP2_LUT_SIZE = 1.0f / EXP2_LUT_SIZE_F;
constexpr float EXP2_LUT_MIN = -20.0f;
constexpr float EXP2_LUT_MAX = 10.0f;
constexpr float EXP2_LUT_RANGE = EXP2_LUT_MAX - EXP2_LUT_MIN;
constexpr float EXP2_LUT_RANGE_INV = 1.0f / EXP2_LUT_RANGE;

// Feedback
constexpr uint8_t MAX_FEEDBACK_VALUE = 7;
constexpr float FEEDBACK_TABLE[8] = {
    0.0f, 0.015625f, 0.03125f, 0.0625f, 0.125f, 0.25f, 0.5f, 1.0f
};
constexpr float FEEDBACK_SCALING = 1.0f;

// Operator
constexpr float OPERATOR_SCALING = 0.125f;

// Keyboard level scaling curves
constexpr uint8_t KEYSCALE_LINEAR[100] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99
};

constexpr uint8_t KEYSCALE_EXP[33] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 14, 16, 19, 23, 27, 33, 39, 47, 56, 66,
    80, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 250
};

// Frequency
constexpr float DETUNE_SCALING = 0.07f;
constexpr uint8_t MAX_VELOCITY_SENSITIVITY = 7;
constexpr uint8_t MAX_DETUNE = 14;
constexpr uint8_t MAX_COARSE = 31;
constexpr uint8_t MAX_FINE = 99;

constexpr float FIXED_FREQ_BASE[4] = {1.0f, 10.0f, 100.0f, 1000.0f};

constexpr float FIXED_FREQ_FINE_VALUES[100] = {
    1.0f,       1.02329f,   1.04713f,   1.07152f,   1.09648f,   1.12202f,   1.14815f,   1.17490f,   1.20226f,   1.23027f,
    1.25893f,   1.28825f,   1.31826f,   1.34896f,   1.38038f,   1.41254f,   1.44544f,   1.47911f,   1.51356f,   1.54882f,
    1.58489f,   1.62181f,   1.65959f,   1.69824f,   1.73780f,   1.77826f,   1.81970f,   1.86209f,   1.90546f,   1.94984f,
    1.99526f,   2.04174f,   2.08930f,   2.13796f,   2.18776f,   2.23872f,   2.29087f,   2.34423f,   2.39883f,   2.45475f,
    2.51189f,   2.57040f,   2.63027f,   2.69153f,   2.75423f,   2.81838f,   2.88404f,   2.95123f,   3.01999f,   3.09032f,
    3.16228f,   3.23594f,   3.31131f,   3.38844f,   3.46737f,   3.54813f,   3.63078f,   3.71535f,   3.80189f,   3.89045f,
    3.98107f,   4.07380f,   4.16869f,   4.26582f,   4.36516f,   4.46684f,   4.57088f,   4.67735f,   4.78630f,   4.89779f,
    5.01187f,   5.12859f,   5.24808f,   5.37032f,   5.49541f,   5.62341f,   5.75440f,   5.88844f,   6.02559f,   6.16595f,
    6.30957f,   6.45654f,   6.60693f,   6.76083f,   6.91831f,   7.07946f,   7.24436f,   7.41311f,   7.58578f,   7.76247f,
    7.94328f,   8.12830f,   8.31764f,   8.51138f,   8.70964f,   8.91251f,   9.12011f,   9.33253f,   9.54993f,   9.77237f
};

// DX7 detune table (non-linear)
constexpr float DETUNE_TABLE[15] = {
    0.0f, 0.078f, 0.156f, 0.234f, 0.312f, 0.468f, 0.624f, 0.780f, 
    0.936f, 1.092f, 1.248f, 1.404f, 1.560f, 1.872f, 2.184f
};

// Envelope
constexpr uint32_t Q24_ONE = 1 << 24;
constexpr float INV_Q24_ONE = 1.0f / static_cast<float>(Q24_ONE);
constexpr int LG_N = 6;
constexpr int N = (1 << LG_N);

// Velocity sensitivity
constexpr float VELOCITY_FACTOR_TABLE[8][9] = {
    {0.543250331f,  0.543250331f,   0.543250331f,   0.543250331f,   0.543250331f,   0.543250331f,   0.543250331f,   0.543250331f,   0.543250331f    },
    {0.595662144f,  0.568852931f,   0.543250331f,   0.501187234f,   0.45708819f,    0.421696503f,   0.3672823f,     0.309029543f,   0.154881662f    },
    {0.647888095f,  0.595662144f,   0.518800039f,   0.45708819f,    0.384591782f,   0.323593657f,   0.251188643f,   0.177827941f,   0.042657952f    },
    {0.691830971f,  0.623734835f,   0.518800039f,   0.421696503f,   0.323593657f,   0.251188643f,   0.169824365f,   0.096605088f,   0.011885022f    },
    {0.770016444f,  0.651628394f,   0.501187234f,   0.384591782f,   0.27542287f,    0.1840772f,     0.114815362f,   0.054954087f,   0.003427678f    },
    {0.839459987f,  0.677641508f,   0.501187234f,   0.354813389f,   0.229086765f,   0.142889396f,   0.077624712f,   0.031622777f,   0.001188502f    },
    {0.920449572f,  0.706317554f,   0.478630092f,   0.323593657f,   0.192752491f,   0.10964782f,    0.053088444f,   0.017378008f,   0.000524807f    },
    {1.0f,          0.73790423f,    0.478630092f,   0.298538262f,   0.16218101f,    0.086099375f,   0.035892193f,   0.01f,          0.000398107f    }
};
constexpr int VELOCITY_POINTS[9] = {127, 111, 95, 79, 64, 48, 32, 16, 1};

// LFO
constexpr float LFO_SPEED[100] = {
    0.062541f,  0.125031f,  0.312393f,  0.437120f,  0.624610f,
    0.750694f,  0.936330f,  1.125302f,  1.249609f,  1.436782f,
    1.560915f,  1.752081f,  1.875117f,  2.062494f,  2.247191f,
    2.374451f,  2.560492f,  2.686728f,  2.873976f,  2.998950f,
    3.188013f,  3.369840f,  3.500175f,  3.682224f,  3.812065f,
    4.000800f,  4.186202f,  4.310716f,  4.501260f,  4.623209f,
    4.814636f,  4.930480f,  5.121901f,  5.315191f,  5.434783f,
    5.617346f,  5.750431f,  5.946717f,  6.062811f,  6.248438f,
    6.431695f,  6.564264f,  6.749460f,  6.868132f,  7.052186f,
    7.250580f,  7.375719f,  7.556294f,  7.687577f,  7.877738f,
    7.993605f,  8.181967f,  8.372405f,  8.504848f,  8.685079f,
    8.810573f,  8.986341f,  9.122423f,  9.300595f,  9.500285f,
    9.607994f,  9.798158f,  9.950249f,  10.117361f, 11.251125f,
    11.384335f, 12.562814f, 13.676149f, 13.904338f, 15.092062f,
    16.366612f, 16.638935f, 17.869907f, 19.193858f, 19.425019f,
    20.833333f, 21.034918f, 22.502250f, 24.003841f, 24.260068f,
    25.746653f, 27.173913f, 27.578599f, 29.052876f, 30.693677f,
    31.191516f, 32.658393f, 34.317090f, 34.674064f, 36.416606f,
    38.197097f, 38.550501f, 40.387722f, 40.749796f, 42.625746f,
    44.326241f, 44.883303f, 46.772685f, 48.590865f, 49.261084f 
};

constexpr float LFO_DELAY[100] = {
    0.000f, 0.006f, 0.012f, 0.019f, 0.026f, 0.033f, 0.040f, 0.047f, 0.051f, 0.055f,
    0.062f, 0.069f, 0.076f, 0.082f, 0.089f, 0.092f, 0.094f, 0.095f, 0.096f, 0.096f,
    0.106f, 0.116f, 0.126f, 0.136f, 0.138f, 0.139f, 0.140f, 0.141f, 0.141f, 0.141f,
    0.161f, 0.181f, 0.200f, 0.210f, 0.214f, 0.216f, 0.217f, 0.218f, 0.219f, 0.219f,
    0.284f, 0.325f, 0.366f, 0.398f, 0.414f, 0.422f, 0.426f, 0.428f, 0.429f, 0.430f,
    0.486f, 0.526f, 0.571f, 0.606f, 0.631f, 0.643f, 0.649f, 0.652f, 0.654f, 0.656f,
    0.712f, 0.768f, 0.824f, 0.851f, 0.878f, 0.892f, 0.899f, 0.902f, 0.904f, 0.906f, 
    1.017f, 1.092f, 1.167f, 1.236f, 1.271f, 1.288f, 1.296f, 1.301f, 1.303f, 1.305f,
    1.455f, 1.562f, 1.670f, 1.734f, 1.766f, 1.782f, 1.789f, 1.793f, 1.795f, 1.797f,
    1.932f, 2.080f, 2.153f, 2.190f, 2.227f, 2.375f, 2.523f, 2.615f, 2.724f, 2.832f 
};

constexpr float LFO_PMS[8] = {
    0.0f, 0.051f, 0.092f, 0.135f, 0.21f, 0.355f, 0.615f, 1.000f
};

// Pitch envelope rate table (from Dexed)
constexpr uint8_t PITCHENV_RATE[100] = {
    1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12,
    12, 13, 13, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 30, 31, 33, 34, 36, 37, 38, 39, 41, 42, 44, 46, 47,
    49, 51, 53, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 79, 82,
    85, 88, 91, 94, 98, 102, 106, 110, 115, 120, 125, 130, 135, 141, 147,
    153, 159, 165, 171, 178, 185, 193, 202, 211, 232, 243, 254, 255
};

// Pitch envelope level table (from Dexed) - maps 0-99 to log scale
constexpr int8_t PITCHENV_TAB[100] = {
    -128, -116, -104, -95, -85, -76, -68, -61, -56, -52, -49, -46, -43,
    -41, -39, -37, -35, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24,
    -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10,
    -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 33, 34, 35, 38, 40, 43, 46, 49, 53, 58, 65, 73,
    82, 92, 103, 115, 127
};

// Parameters constants
// Default file path for global parameters persistence
// On Teensy, this would be on SD card; on PC, in current directory
constexpr const char* PARAMS_FILE_PATH = "params.bin";
constexpr uint8_t PARAMS_VERSION = 1;
constexpr uint32_t PARAMS_MAGIC = 0x47504152; // "GPAR"

// Inverse constants for parameter normalization
constexpr float INV_PARAM_99 = 1.0f / 99.0f;
constexpr float INV_PARAM_7 = 1.0f / 7.0f;
constexpr float INV_PARAM_3 = 1.0f / 3.0f;

#endif // CONSTANTS_H