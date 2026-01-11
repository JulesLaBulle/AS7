#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include "config.h"

// -----------------------------------------------------------------------------
// Predefined FM Algorithms for Yamaha DX7
// Total 32 algorithms as per the original hardware specifications
// Each algorithm includes feedback configuration
// -----------------------------------------------------------------------------

namespace Algorithms {

// -----------------------------------------------------------------------------
// Algorithm 1:
// op2 → op1
// op6 feedback
// op6 → op5 → op4 → op3
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_1 = []() {
    AlgorithmConfig config;
    
    // Connection matrix
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier, mod by OP2)
        { true,  false, false, false, false, false }, // OP2 → OP1 (modulator)
        { false, false, false, false, false, false }, // OP3 (carrier, end of chain)
        { false, false, true,  false, false, false }, // OP4 → OP3 (modulator)
        { false, false, false, true,  false, false }, // OP5 → OP4 (modulator)
        { false, false, false, false, true,  false }  // OP6 → OP5 (modulator)
    };
    
    // Copy connection matrix
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    // Modulator counts
    uint8_t counts[6] = {1, 0, 1, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    // Modulator indices
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators (feedback is handled separately)
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    // Carrier flags (OP1, OP3 are carriers)
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    // Feedback configuration
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 2:
// op2 feedback
// op2 → op1
// op6 → op5 → op4 → op3
// Feedback on OP2
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_2 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators (feedback is separate)
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 1; // OP2 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 3:
// op3 → op2 → op1
// op6 feedback
// op6 → op5 → op4
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_3 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 4:
// op3 → op2 → op1
// op6 → op5 → op4
// op4 → op6 (modulation, not feedback)
// No feedback in this algorithm
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_4 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, true  }, // OP4 → OP6
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 1, 1, 1};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {3, 0, 0, 0, 0, 0}  // OP6 modulated by OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = false;
    config.feedbackOperator = 0; // No feedback in this algorithm
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 5:
// op2 → op1
// op4 → op3
// op6 feedback
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_5 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 6:
// op2 → op1
// op4 → op3
// op6 → op5
// op5 → op6 (feedback modulation, not self-feedback)
// No traditional feedback in this algorithm
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_6 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, false, false, true  }, // OP5 → OP6
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 0, 1, 1};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {4, 0, 0, 0, 0, 0}  // OP6 modulated by OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = false;
    config.feedbackOperator = 0; // No self-feedback in this algorithm
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 7:
// op2 → op1
// op4 → op3
// op6 feedback
// op6 → op5 → op3
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_7 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, true,  false, false, false }, // OP5 → OP3
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 2, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 4, 0, 0, 0, 0}, // OP3 modulated by OP4 and OP5
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 8:
// op4 feedback
// op2 → op1
// op4 → op3
// op6 → op5 → op3
// Feedback on OP4
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_8 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, true,  false, false, false }, // OP5 → OP3
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 2, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 4, 0, 0, 0, 0}, // OP3 modulated by OP4 and OP5
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 3; // OP4 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 9:
// op2 feedback
// op2 → op1
// op4 → op3
// op6 → op5 → op3
// Feedback on OP2
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_9 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, true,  false, false, false }, // OP5 → OP3
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 2, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 4, 0, 0, 0, 0}, // OP3 modulated by OP4 and OP5
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 1; // OP2 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 10:
// op5 → op4
// op6 → op4
// op3 feedback
// op3 → op2 → op1
// Feedback on OP3
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_10 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 2, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 2; // OP3 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 11:
// op5 → op4
// op6 → op4
// op6 feedback
// op3 → op2 → op1
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_11 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 2, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 12:
// op4 → op3
// op5 → op3
// op6 → op3
// op2 feedback
// op2 → op1
// Feedback on OP2
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_12 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, true,  false, false, false }, // OP5 → OP3
        { false, false, true,  false, false, false }  // OP6 → OP3
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 3, 0, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 4, 5, 0, 0, 0}, // OP3 modulated by OP4, OP5, OP6
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 1; // OP2 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 13:
// op4 → op3
// op5 → op3
// op6 → op3
// op6 feedback
// op2 → op1
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_13 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, true,  false, false, false }, // OP5 → OP3
        { false, false, true,  false, false, false }  // OP6 → OP3
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 3, 0, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 4, 5, 0, 0, 0}, // OP3 modulated by OP4, OP5, OP6
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 14:
// op6 feedback
// op2 → op1
// op5 and op6 → op4 → op3
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_14 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 2, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6 (note: OP5 doesn't actually have a modulator)
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 15:
// op2 feedback
// op2 → op1
// op5 and op6 → op4 → op3
// Feedback on OP2
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_15 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 2, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 1; // OP2 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 16:
// op6 feedback
// op6 → op5 → op1
// op4 → op3 → op1
// op2 → op1
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_16 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { true,  false, false, false, false, false }, // OP3 → OP1
        { false, false, true,  false, false, false }, // OP4 → OP3
        { true,  false, false, false, false, false }, // OP5 → OP1
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {3, 0, 1, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 2, 4, 0, 0, 0}, // OP1 modulated by OP2, OP3, OP5
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 17:
// op2 feedback
// op6 → op5 → op1
// op4 → op3 → op1
// op2 → op1
// Feedback on OP2
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_17 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { true,  false, false, false, false, false }, // OP3 → OP1
        { false, false, true,  false, false, false }, // OP4 → OP3
        { true,  false, false, false, false, false }, // OP5 → OP1
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {3, 0, 1, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 2, 4, 0, 0, 0}, // OP1 modulated by OP2, OP3, OP5
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 1; // OP2 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 18:
// op2 → op1
// op3 feedback
// op3 → op1
// op6 → op5 → op4 → op1
// Feedback on OP3
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_18 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { true,  false, false, false, false, false }, // OP3 → OP1
        { true,  false, false, false, false, false }, // OP4 → OP1
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {3, 0, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 2, 3, 0, 0, 0}, // OP1 modulated by OP2, OP3, OP4
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, false, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 2; // OP3 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 19:
// op3 → op2 → op1
// op6 feedback
// op6 → op4
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_19 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, true,  true,  false }  // OP6 → OP4 and OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, false, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 20:
// op3 feedback
// op3 → op1
// op3 → op2
// op5 and op6 → op4
// Feedback on OP3
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_20 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { true,  true,  false, false, false, false }, // OP3 → OP1 and OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 2, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {2, 0, 0, 0, 0, 0}, // OP1 modulated by OP3
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 2; // OP3 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 21:
// op3 feedback
// op3 → op1
// op3 → op2
// op6 → op4
// op6 → op5
// Feedback on OP3
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_21 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { true,  true,  false, false, false, false }, // OP3 → OP1 and OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, true,  true,  false }  // OP6 → OP4 and OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 1, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {2, 0, 0, 0, 0, 0}, // OP1 modulated by OP3
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, false, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 2; // OP3 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 22:
// op2 → op1
// op6 feedback
// op6 → op3
// op6 → op4
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_22 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, true,  true,  true,  false }  // OP6 → OP3, OP4, OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP3 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 23:
// op1 non modulé (carrier)
// op3 → op2
// op6 feedback
// op6 → op4
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_23 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, true,  true,  false }  // OP6 → OP4 and OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 1, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, false, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 24:
// op1, op2 non modulés (carriers)
// op6 feedback
// op6 → op3
// op6 → op4
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_24 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, true,  true,  true,  false }  // OP6 → OP3, OP4, OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 1, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP3 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 25:
// op1, op2, op3 non modulés (carriers)
// op6 feedback
// op6 → op4
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_25 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, true,  true,  false }  // OP6 → OP4 and OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 0, 1, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP4 modulated by OP6
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 26:
// op1 non modulé (carrier)
// op3 → op2
// op6 feedback
// op6 and op5 → op4
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_26 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 1, 0, 2, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 27:
// op1 non modulé (carrier)
// op3 feedback
// op3 → op2
// op5 and op6 → op4
// Feedback on OP3
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_27 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, true,  false, false, false, false }, // OP3 → OP2
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, true,  false, false }  // OP6 → OP4
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 1, 0, 2, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {2, 0, 0, 0, 0, 0}, // OP2 modulated by OP3
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {4, 5, 0, 0, 0, 0}, // OP4 modulated by OP5 and OP6
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, false, true, false, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 2; // OP3 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 28:
// op2 → op1
// op5 feedback
// op5 → op4 → op3
// op6 non modulé (carrier)
// Feedback on OP5
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_28 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { true,  false, false, false, false, false }, // OP2 → OP1
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, false, false }  // OP6 (carrier)
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {1, 0, 1, 1, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {1, 0, 0, 0, 0, 0}, // OP1 modulated by OP2
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, false, true, false, false, true};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 4; // OP5 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 29:
// op1 and op2 non modulés (carriers)
// op4 → op3
// op6 feedback
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_29 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 1, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, false, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 30:
// op1, op2, op6 non modulés (carriers)
// op5 feedback
// op5 → op4 → op3
// Feedback on OP5
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_30 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, true,  false, false, false }, // OP4 → OP3
        { false, false, false, true,  false, false }, // OP5 → OP4
        { false, false, false, false, false, false }  // OP6 (carrier)
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 1, 1, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {3, 0, 0, 0, 0, 0}, // OP3 modulated by OP4
        {4, 0, 0, 0, 0, 0}, // OP4 modulated by OP5
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, false, false, true};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 4; // OP5 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 31:
// op1, op2, op3, op4 non modulés (carriers)
// op6 feedback
// op6 → op5
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_31 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, false, true,  false }  // OP6 → OP5
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 0, 0, 1, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {5, 0, 0, 0, 0, 0}, // OP5 modulated by OP6
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, true, true, false};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// -----------------------------------------------------------------------------
// Algorithm 32:
// All operators are carriers
// Only op6 has feedback (self-modulation)
// Feedback on OP6
// -----------------------------------------------------------------------------
static const AlgorithmConfig ALGORITHM_32 = []() {
    AlgorithmConfig config;
    
    bool conn[6][6] = {
        { false, false, false, false, false, false }, // OP1 (carrier)
        { false, false, false, false, false, false }, // OP2 (carrier)
        { false, false, false, false, false, false }, // OP3 (carrier)
        { false, false, false, false, false, false }, // OP4 (carrier)
        { false, false, false, false, false, false }, // OP5 (carrier)
        { false, false, false, false, false, false }  // OP6 (carrier, no connections)
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.connections[i][j] = conn[i][j];
    
    uint8_t counts[6] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 6; ++i)
        config.modulatorCount[i] = counts[i];
    
    uint8_t indices[6][6] = {
        {0, 0, 0, 0, 0, 0}, // OP1 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP2 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP3 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP4 has no modulators
        {0, 0, 0, 0, 0, 0}, // OP5 has no modulators
        {0, 0, 0, 0, 0, 0}  // OP6 has no modulators
    };
    
    for (int i = 0; i < 6; ++i)
        for (int j = 0; j < 6; ++j)
            config.modulatorIndices[i][j] = indices[i][j];
    
    bool carriers[6] = {true, true, true, true, true, true};
    for (int i = 0; i < 6; ++i)
        config.isCarrier[i] = carriers[i];
    
    config.hasFeedback = true;
    config.feedbackOperator = 5; // OP6 has feedback
    
    return config;
}();

// Array of all 32 DX7 algorithms
static constexpr size_t NUM_ALGORITHMS = 32;
static const AlgorithmConfig* ALL_ALGORITHMS[NUM_ALGORITHMS] = {
    &ALGORITHM_1,  &ALGORITHM_2,  &ALGORITHM_3,  &ALGORITHM_4,
    &ALGORITHM_5,  &ALGORITHM_6,  &ALGORITHM_7,  &ALGORITHM_8,
    &ALGORITHM_9,  &ALGORITHM_10, &ALGORITHM_11, &ALGORITHM_12,
    &ALGORITHM_13, &ALGORITHM_14, &ALGORITHM_15, &ALGORITHM_16,
    &ALGORITHM_17, &ALGORITHM_18, &ALGORITHM_19, &ALGORITHM_20,
    &ALGORITHM_21, &ALGORITHM_22, &ALGORITHM_23, &ALGORITHM_24,
    &ALGORITHM_25, &ALGORITHM_26, &ALGORITHM_27, &ALGORITHM_28,
    &ALGORITHM_29, &ALGORITHM_30, &ALGORITHM_31, &ALGORITHM_32
};

} // namespace Algorithms

#endif // CONNECTIONS_H