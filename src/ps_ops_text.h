#pragma once

#include "pscore.h"
#include "ps_type_font.h"
#include "psvm.h"

namespace waavs {
    inline bool op_ashow(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();

        PSObject axObj;
        PSObject ayObj;
        PSObject strObj;

        if (!ostk.pop(strObj) ||
            !ostk.pop(ayObj) ||
            !ostk.pop(axObj)) {
            return vm.error("op_ashow: stackunderflow");
        }

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
            { "ashow",     op_ashow }
            ,{"show", op_show}

        };
        return table;
    }
}