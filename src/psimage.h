#pragma once


#include <vector>

#include "psmatrix.h"

namespace waavs {
    struct PSImage {
        int width;
        int height;
        int bitsPerComponent;
        PSMatrix transform;
        std::vector<uint8_t> data;
    };
}