#pragma once

#include <vector>
#include <memory>
#include <deque>

#include "ps_type_matrix.h"
#include "ps_type_paint.h"
#include "ps_type_path.h"
#include "ps_type_font.h"

namespace waavs {

    enum class PSLineCap : int {
        Butt = 0,
        Round = 1,
        Square = 2
    };

    enum class PSLineJoin : int {
        Miter = 0,
        Round = 1,
        Bevel = 2
    };



} // namespace waavs

namespace waavs {

    struct PSGraphicsState {
        // Current Transformation Matrix
        PSMatrix ctm = PSMatrix::makeIdentity();

        // Stroke Style
        double lineWidth = 1.0;
        double miterLimit = 10.0;
        PSLineCap lineCap = PSLineCap::Butt;   // 0 = butt, 1 = round, 2 = square
        PSLineJoin lineJoin = PSLineJoin::Miter;  // 0 = miter, 1 = round, 2 = bevel

        // Flatness 
        double flatness = 1.0;

        // Dash pattern for stroking paths
        double dashOffset = 0.0;
        std::vector<double> dashArray;

        // Current font
        PSObject fCurrentFont; // Handle to the current font

        // Clipping path (not implemented yet — placeholder)
        // May need a region or path representation later
        bool hasClip = false;
        PSPath fCurrentClipPath; // Current clipping path
        PSPath fCurrentPath;

        // Paint
        PSPaint strokePaint = PSPaint::fromGray(0.0); // Default: black
        PSPaint fillPaint = PSPaint::fromGray(0.0);   // Default: black

        
        
        // Constructors
        PSGraphicsState() = default;

        PSGraphicsState(const PSGraphicsState& other) = default;
        PSGraphicsState& operator=(const PSGraphicsState& other) = default;

        double getLineWidth() const { return lineWidth; }
        double getMiterLimit() const { return miterLimit; }
        double getFlatness() const { return flatness; }
        PSLineCap getLineCap() const { return lineCap; }
        PSLineJoin getLineJoin() const { return lineJoin; }
        const std::vector<double>& getDashArray() const { return dashArray; }
        double getDashOffset() const { return dashOffset; }
        PSFontHandle getFont() const { return fCurrentFont.asFont(); }
    };


    struct PSStateMemoryPool {
        std::vector<std::unique_ptr<PSGraphicsState>> pool;
        size_t currentIndex = 0;

        PSStateMemoryPool(size_t initialSize = 16) {
            pool.reserve(initialSize);
            for (size_t i = 0; i < initialSize; ++i)
                pool.push_back(std::make_unique<PSGraphicsState>());
        }

        PSGraphicsState* allocate() {
            if (currentIndex >= pool.size()) {
                size_t oldSize = pool.size();
                size_t newSize = oldSize * 2;
                pool.reserve(newSize);
                for (size_t i = oldSize; i < newSize; ++i)
                    pool.push_back(std::make_unique<PSGraphicsState>());
            }
            return pool[currentIndex++].get();
        }

        void reset() { currentIndex = 0; }
    };


    struct PSGraphicsStack {
        std::deque<PSGraphicsState*> stack;
        std::unique_ptr<PSGraphicsState> current;
        std::unique_ptr<PSStateMemoryPool> pool;

        PSGraphicsStack()
            : current(std::make_unique<PSGraphicsState>()),
            pool(std::make_unique<PSStateMemoryPool>()) {
        }

        // Return the current state (non-null)
        PSGraphicsState* get() const { return current.get(); }

        // Push current state onto the stack (gsave)
        void gsave() {
            PSGraphicsState* newState = pool->allocate();
            *newState = *current; // shallow copy
            stack.push_back(newState);
        }

        // Pop previous state from the stack (grestore)
        void grestore() {
            if (!stack.empty()) {
                current = std::make_unique<PSGraphicsState>(*stack.back());
                stack.pop_back();
            }
            else {
                printf("PSGraphicsStack::grestore(): stack underflow\n");
            }
        }

        void reset() {
            stack.clear();
            pool->reset();
            current = std::make_unique<PSGraphicsState>();
        }

        bool empty() const { return stack.empty(); }
        size_t depth() const { return stack.size(); }
    };

} // namespace waavs

