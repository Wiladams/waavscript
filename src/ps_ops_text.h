#pragma once

#include "pscore.h"
#include "ps_type_font.h"
#include "psvm.h"

namespace waavs {
    inline bool op_ashow(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        auto* g = vm.graphics();
        auto& ctm = g->getCTM();

        PSObject axObj;
        PSObject ayObj;
        PSObject strObj;

        if (!ostk.pop(strObj) ||
            !ostk.pop(ayObj) ||
            !ostk.pop(axObj)) {
            return vm.error("op_ashow: stackunderflow");
        }

        // BUGBUG: not using specified adjustments
        g->showText(ctm, strObj.asMutableString());

        return true;
    }

    inline bool op_kshow(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* g = vm.graphics();
        auto& ctm = g->getCTM();

        if (ostk.size() < 2)
            return vm.error("op_kshow: stackunderflow");

        // pop a string, then a procedure
        PSObject strObj;
        PSObject proc;

        if (!ostk.pop(strObj) || !strObj.isString())
            return vm.error("op_kshow: typecheck; string");

        if (!ostk.pop(proc) || !proc.isExecutable())
            return vm.error("op_kshow: typecheck; proc");

        // BUGBUG: need to apply per character-pair spacing
        // but for now we'll just do what show does
        g->showText(ctm, strObj.asMutableString());

        return true;
    }

    inline bool op_show(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* g = vm.graphics();
        auto &ctm = g->getCTM();

        if (ostk.empty())
            return vm.error("op_show: stackunderflow");

        PSObject strObj;

        ostk.pop(strObj);

        // pop a string, then render it using current position and font
        g->showText(ctm, strObj.asMutableString());

        return true;
    }

    // Text operator registration
    inline const PSOperatorFuncMap& getTextOps() {
        static const PSOperatorFuncMap table = {
            { "ashow",      op_ashow },
            {"show",        op_show},
            { "kshow",      op_kshow  }
        };
        return table;
    }
}