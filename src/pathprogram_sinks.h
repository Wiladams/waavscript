// pathprogram_sinks.h
#pragma once

#include "pathprogram.h"
#include "pathprogram_builder.h"
#include "pathprogram_flattener.h"
#include "pathprogram_fill_preparer.h"

namespace waavs
{
    inline bool flattenPathProgram(const PathProgram& src, PathProgram& dst, double flatness = 0.25, int maxDepth = 16) noexcept
    {
        PathProgramBuilder builder;
        builder.reserve(src.ops.size() * 2u, src.args.size() * 4u);

        PathFlattener<PathProgramBuilder> flattener(builder);
        flattener.opt.flatness = flatness;
        flattener.opt.maxDepth = maxDepth;

        if (!pathprogram_dispatch(src, flattener))
            return false;

        dst = std::move(builder.prog);
        return true;
    }

    inline bool flattenPathForFill(const PathProgram& src, PathProgram& dst, double flatness = 0.25, int maxDepth = 16) noexcept
    {
        PathProgramBuilder builder;
        PathFillPreparer<PathProgramBuilder> preparer(builder);
        PathFlattener<decltype(preparer)> flattener(preparer);

        flattener.opt.flatness = flatness;
        flattener.opt.maxDepth = maxDepth;

        if (!pathprogram_dispatch(src, flattener))
            return false;
        
        // Live paths may not contain an explicit OP_END.
        // Flush the final open contour and finalize the builder.
        if (src.ops.empty() || src.ops.back() != OP_END) {
            if (!flattener.onEnd())
                return false;
        }

        dst = std::move(builder.prog);
        return true;
    }
}