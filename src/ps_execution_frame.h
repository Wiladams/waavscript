// ps_execution_frame.h
#pragma once



#include "pscore.h"
#include "ps_scanner.h"

#include <cstddef>
#include <cstdint>
#include <variant>
#include <memory>


namespace waavs
{
    struct PSScheduledObject
    {
        PSObject object;
    };

    struct PSImageFrame
    {
        int32_t width = 0;
        int32_t height = 0;
        int32_t bitsPerComponent = 0;

        PSObject matrix;
        PSObject procedure;

        std::shared_ptr<PSString> data;
        size_t written = 0;
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

    struct PSPathForAllFrame
    {
        PSObject path;

        PSObject moveProc;
        PSObject lineProc;
        PSObject curveProc;
        PSObject closeProc;

        size_t opIndex = 0;
        size_t argIndex = 0;

        PSMatrix inverseCTM;
    };

    struct PSFileFrame
    {
        PSFileHandle file;
        std::shared_ptr<PSObjectGenerator> generator;
    };

    // For showing text with adjustments, 
    // we need to keep track of the string 
    // and the procedure
    struct PSKShowFrame
    {
        PSObject string;
        PSObject procedure;
        size_t index = 0;
    };

    // For cleaning up after eexec
    struct PSEexecFrame
    {};

    struct PSStoppedFrame
    {};

    using PSExecutionItem = std::variant <
        PSScheduledObject,
        PSFileFrame,
        PSEexecFrame,
        PSImageFrame,
        PSProcedureFrame,
        PSRepeatFrame,
        PSLoopFrame,
        PSForFrame,
        PSForAllFrame,
        PSResourceForAllFrame,
        PSPathForAllFrame,
        PSKShowFrame,
        PSStoppedFrame
    >;
}