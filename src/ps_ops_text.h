// ps_ops_text.h
#pragma once

#include "pscore.h"
#include "ps_type_font.h"
#include "psvm.h"

namespace waavs 
{

    inline bool op_ashow(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.size() < 3)
            return vm.error("op_ashow: stackunderflow");

        PSObject strObj;
        PSObject ayObj;
        PSObject axObj;

        if (!ostk.pop(strObj) || !strObj.isString())
            return vm.error("op_ashow: typecheck; expected string");

        if (!ostk.pop(ayObj) || !ayObj.isNumber())
            return vm.error("op_ashow: typecheck; expected number for ay");

        if (!ostk.pop(axObj) || !axObj.isNumber())
            return vm.error("op_ashow: typecheck; expected number for ax");

        const double ax = axObj.asReal();
        const double ay = ayObj.asReal();

        const PSString& str = strObj.asString();
        const uint8_t* data = str.data();
        const size_t length = str.length();

        for (size_t i = 0; i < length; ++i)
        {
            const PSMatrix& ctm = grph->getCTM();

            if (!grph->showText(ctm, data + i, 1))
                return vm.error("op_ashow: showText failed");

            if (!grph->currentPath().rsetCurrentPoint(ctm, ax, ay))
                return vm.error("op_ashow: current point adjustment failed");
        }

        return true;
    }

    inline bool op_awidthshow(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.size() < 6)
            return vm.error("op_awidthshow: stackunderflow");

        PSObject strObj;
        PSObject ayObj;
        PSObject axObj;
        PSObject charObj;
        PSObject cyObj;
        PSObject cxObj;

        if (!ostk.pop(strObj) || !strObj.isString())
            return vm.error("op_awidthshow: typecheck; expected string");

        if (!ostk.pop(ayObj) || !ayObj.isNumber())
            return vm.error("op_awidthshow: typecheck; expected number for ay");

        if (!ostk.pop(axObj) || !axObj.isNumber())
            return vm.error("op_awidthshow: typecheck; expected number for ax");

        if (!ostk.pop(charObj) || charObj.type != PSObjectType::Int)
            return vm.error("op_awidthshow: typecheck; expected integer character code");

        if (!ostk.pop(cyObj) || !cyObj.isNumber())
            return vm.error("op_awidthshow: typecheck; expected number for cy");

        if (!ostk.pop(cxObj) || !cxObj.isNumber())
            return vm.error("op_awidthshow: typecheck; expected number for cx");

        const int32_t charCode = charObj.asInt();
        if (charCode < 0 || charCode > 255)
            return vm.error("op_awidthshow: rangecheck");

        const double cx = cxObj.asReal();
        const double cy = cyObj.asReal();
        const double ax = axObj.asReal();
        const double ay = ayObj.asReal();

        const PSString& str = strObj.asString();
        const uint8_t* data = str.data();
        const size_t length = str.length();

        for (size_t i = 0; i < length; ++i)
        {
            const PSMatrix& ctm = grph->getCTM();
            const uint8_t ch = data[i];

            if (!grph->showText(ctm, data + i, 1))
                return vm.error("op_awidthshow: showText failed");

            if (!grph->currentPath().rsetCurrentPoint(ctm, ax, ay))
                return vm.error("op_awidthshow: base current point adjustment failed");

            if (ch == static_cast<uint8_t>(charCode))
            {
                if (!grph->currentPath().rsetCurrentPoint(ctm, cx, cy))
                    return vm.error("op_awidthshow: conditional current point adjustment failed");
            }
        }

        return true;
    }


    inline bool op_kshow(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_kshow: stackunderflow");

        PSObject strObj;
        PSObject proc;

        if (!s.pop(strObj) || !strObj.isString())
            return vm.error("op_kshow: typecheck; string");

        if (!s.pop(proc) || !proc.isArray() || !proc.isExecutable())
            return vm.error("op_kshow: typecheck; proc");

        return vm.scheduleKShow(strObj, proc);
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

    inline bool op_widthshow(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.size() < 4)
            return vm.error("op_widthshow: stackunderflow");

        PSObject strObj;
        PSObject charObj;
        PSObject cyObj;
        PSObject cxObj;

        if (!ostk.pop(strObj) || !strObj.isString())
            return vm.error("op_widthshow: typecheck; expected string");

        if (!ostk.pop(charObj) || charObj.type != PSObjectType::Int)
            return vm.error("op_widthshow: typecheck; expected integer character code");

        if (!ostk.pop(cyObj) || !cyObj.isNumber())
            return vm.error("op_widthshow: typecheck; expected number for cy");

        if (!ostk.pop(cxObj) || !cxObj.isNumber())
            return vm.error("op_widthshow: typecheck; expected number for cx");

        const int32_t charCode = charObj.asInt();
        if (charCode < 0 || charCode > 255)
            return vm.error("op_widthshow: rangecheck");

        const double cx = cxObj.asReal();
        const double cy = cyObj.asReal();

        const PSString& str = strObj.asString();
        const uint8_t* data = str.data();
        const size_t length = str.length();

        for (size_t i = 0; i < length; ++i)
        {
            const PSMatrix& ctm = grph->getCTM();
            const uint8_t ch = data[i];

            if (!grph->showText(ctm, data + i, 1))
                return vm.error("op_widthshow: showText failed");

            if (ch == static_cast<uint8_t>(charCode))
            {
                if (!grph->currentPath().rsetCurrentPoint(ctm, cx, cy))
                    return vm.error("op_widthshow: current point adjustment failed");
            }
        }

        return true;
    }

    // Text operator registration
    inline const PSOperatorFuncMap& getTextOps() {
        static const PSOperatorFuncMap table = {
            { "ashow",      op_ashow },
            { "awidthshow", op_awidthshow },
            { "show",        op_show},
            { "kshow",      op_kshow  },
            { "widthshow",  op_widthshow }
        };
        return table;
    }
}