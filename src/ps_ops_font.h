#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    inline bool op_findfont(PSVirtualMachine& vm) {
        auto *g = vm.graphics();
        auto& s = vm.opStack();

        PSObject param;
        if (!s.pop(param))
            return vm.error("stackunderflow");
        
        // There are two cases for findfont:
        // 1. A name is on the stack (e.g., "Helvetica")
        // 2. A dictionary is on the stack (e.g., a font dictionary)

        // For now, we'll only deal with the first case

        if (!param.isName())
            return vm.error("typecheck: expected name for findfont");

        // Query FontMonger for the fonthandle associated with the name
        PSObject fontFace;
        if (!g->findFont(param.asName(), fontFace))
            return vm.error("failed to find font");

        // Push the font object onto the stack
        return s.push(fontFace);
    }

    inline bool op_scalefont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject size, fontFace;
        if (!s.pop(size) || !size.isNumber())
            return vm.error("typecheck: expected number");
        if (!s.pop(fontFace) || !fontFace.isFontFace())
            return vm.error("typecheck: expected font face");

        auto fontHandle = PSFont::createFromSize(fontFace.asFontFace(), size.asReal());

        if (!fontHandle) {
            return vm.error("invalidfont: failed to create font from size");
        }

        return s.push(PSObject::fromFont(fontHandle));
    }

    inline bool op_makefont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject matrix, font;
        if (!s.pop(matrix) || !matrix.isArray())
            return vm.error("typecheck: expected array (matrix)");
        if (!s.pop(font) || !font.isFont())
            return vm.error("typecheck: expected font");

        // Stub: Return a transformed font object
        return vm.error("invalidfont: makefont not implemented");
    }

    inline bool op_setfont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject font;
        if (!s.pop(font) || !font.isFont())
            return vm.error("typecheck: expected font");

        vm.graphics()->setFont(font.asFont());
        return true;
    }

    inline bool op_currentfont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        s.push(PSObject::fromFont(vm.graphics()->currentFont()));
        return true;
    }

    inline bool op_definefont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject name, fontDict;
        if (!s.pop(fontDict) || !fontDict.isDictionary())
            return vm.error("typecheck: expected dictionary");
        if (!s.pop(name) || !name.isName())
            return vm.error("typecheck: expected name");

        // Stub: add to FontDirectory
        // fontDirectory().define(name.asName(), PSObject::fromFont(PSFont::create(fontDict)));

        return vm.error("definefont not implemented");
    }

    inline bool op_undefinefont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject name;
        if (!s.pop(name) || !name.isName())
            return vm.error("typecheck: expected name");

        // Stub: remove from FontDirectory
        return vm.error("undefinefont not implemented");
    }

    inline bool op_selectfont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject size, name;
        if (!s.pop(size) || !size.isNumber())
            return vm.error("typecheck: expected number");
        if (!s.pop(name) || !name.isName())
            return vm.error("typecheck: expected name");

        // Equivalent to:
        //   name findfont size scalefont setfont

        // Stub:
        return vm.error("selectfont not implemented");
    }


    // --- Font Operator Registration ---

    inline const PSOperatorFuncMap& getFontOps() {
        static const PSOperatorFuncMap table = {
            { "findfont",     op_findfont },
            { "scalefont",    op_scalefont },
            { "makefont",     op_makefont },
            { "setfont",      op_setfont },
            { "currentfont",  op_currentfont },
            { "definefont",   op_definefont },
            { "undefinefont", op_undefinefont },
            { "selectfont",   op_selectfont }
        };
        return table;
    }
}

