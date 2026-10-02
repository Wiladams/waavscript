// ps_execution_frame.h
#pragma once

#include <cstddef>
#include <cstdint>
#include <variant>

#include "pscore.h"

namespace waavs
{
    struct PSScheduledObject
    {
        PSObject object;
    };

    struct PSProcedureFrame
    {
        PSArrayHandle procedure;
        size_t index = 0;
    };

    struct PSRepeatFrame
    {
        PSObject procedure;
        int32_t remaining = 0;
    };

    struct PSLoopFrame
    {
        PSObject procedure;
    };

    struct PSForFrame
    {
        double current = 0;
        double increment = 0;
        double limit = 0;
        PSObject procedure;
    };

    struct PSForAllFrame
    {
        PSObject container;
        PSObject procedure;
        size_t index = 0;
    };

    struct PSResourceForAllFrame
    {
        PSName category;
        PSObject procedure;

        // iteration state
        size_t resourceIndex = 0;
        size_t entryIndex = 0;
    };

    struct PSStoppedFrame
    {};

    using PSExecutionItem = std::variant<
        PSScheduledObject,
        PSProcedureFrame,
        PSRepeatFrame,
        PSLoopFrame,
        PSForFrame,
        PSForAllFrame,
        PSResourceForAllFrame,
        PSStoppedFrame
    >;
}