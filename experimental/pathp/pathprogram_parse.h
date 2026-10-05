#pragma once

#include <utility>

#include "pathprogram_builder.h"
#include "pathcommand_normalizer.h"
#include "svg_path_reader.h"

namespace waavs
{


    // ------------------------------------------------------------
    // pathProgram_parse()
    //
    // Build a PathProgram from SVG <path> 'd' attribute data.
    //
    // The result is canonicalized and normalized:
    // - no relative commands
    // - no implicit lineto after moveto
    // - arcs are in endpoint form
    // ------------------------------------------------------------

    [[nodiscard]]
    inline bool pathProgram_parse(const MemCursor& input, PathProgram& outProg)
    {
        SVGPathReader reader(input);
        PathProgramBuilder builder;
        PathCommandNormalizer normalizer(builder);

        SVGPathCommand cmd{};
        float args[7]{};
        bool repeated = false;

        for (;;)
        {
            const SVGPathReadResult result = reader.next(cmd, args, repeated);

            if (result == SVGPathReadResult::End)
                break;

            if (result == SVGPathReadResult::Error)
                return false;

            if (!normalizer.consume(cmd, args, repeated))
                return false;
        }

        if (!builder.end())
            return false;

        outProg = std::move(builder.prog);
        return true;
    }

}

