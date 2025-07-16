#pragma once


#include <vector>

#include "ps_type_matrix.h"

namespace waavs {
    struct PSImage {
        int width;
        int height;
        int bitsPerComponent;
        PSMatrix transform;
        std::vector<uint8_t> data;
    };
}