#pragma once

#include "pscore.h"
#include "psvm.h"
#include "fontmonger.h"


namespace waavs {


    inline bool op_findfont(PSVirtualMachine& vm) {
        auto* g = vm.graphics();
        auto& s = vm.opStack();

        PSObject param;
        if (!s.pop(param))
            return vm.error("of_findfont: stackunderflow");

        // There are two cases for findfont:
        // 1. A name is on the stack (e.g., "Helvetica")
        // 2. A dictionary is on the stack (e.g., a font dictionary)

        // For now, we'll only deal with the first case
        if (!param.isName())
            return vm.error("typecheck: expected name for findfont");

        PSName faceName = param.asName();

        // First, lookup the name in the FontMap to see if we have
        // a mapping of the name to a system postscript name
        // a) get the FontMap dictionary from the system
        PSObject fontMapObj;
        if (vm.dictionaryStack.load("FontMap", fontMapObj))
        {
            // try to lookup the systemname
            PSObject systemNameObj;
            if (fontMapObj.asDictionary()->get(param.asName(), systemNameObj))
            {
                if (systemNameObj.isName())
                    faceName = systemNameObj.asName();
            }
        }

        // Query FontMonger for the fonthandle associated with the name
        PSObject fontFace;
        if (!g->findFont(vm, faceName, fontFace))
            return vm.error("failed to find font", faceName.c_str());

        // Push the font object onto the stack
        return s.push(fontFace);
    }

    inline bool op_scalefont(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_scalefont: stackunderflow;");

        PSObject sizeObj;
        PSObject faceObj;

        if (!s.pop(sizeObj) || !sizeObj.isNumber())
            return vm.error("typecheck: expected number");

        if (!s.pop(faceObj) || !faceObj.isFontFace())
            return vm.error("typecheck: expected font face");

        double size = sizeObj.asReal();
        auto face = faceObj.asFontFace();


        PSObject fontObj;
        if (!FontMonger::createFont(faceObj, size, fontObj))
            return vm.error("op_scalefont: createFont failed");

        return s.push(fontObj);
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
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_setfont: stackunderflow;");

        PSObject fontObj;
        if (!ostk.pop(fontObj) || !fontObj.isFont())
            return vm.error("op_setfont: typecheck; expected font");

        vm.graphics()->setFont(fontObj.asFont());

        return true;
    }

    // fontname size selectfont -- font
    // fontname matrix selectfont -- font
    inline bool op_selectfont(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        if (ostk.size() < 2)
            return vm.error("op_selectfont: stackunderflow;");

        PSObject sizeObj;
        PSObject fontNameObj;

        if (!ostk.pop(sizeObj) || !sizeObj.isNumber())
            return vm.error("op_selectfont: typecheck; expected number for size");

        if (!ostk.pop(fontNameObj) || !fontNameObj.isName())
            return vm.error("op_selectfont: typecheck; expected font name");

        // try to find font by name
        ostk.push(fontNameObj); // push name back for findfont
        if (!op_findfont(vm))
            return vm.error("op_selectfont: font not found - ", fontNameObj.asName().c_str());

        PSObject faceObj;
        ostk.pop(faceObj);

        // Now do scale font
        PSObject fontObj;
        if (!FontMonger::createFont(faceObj, sizeObj.asReal(), fontObj))
            return vm.error("op_selectfont: createFont failed");

        // and last, do setfont
        vm.graphics()->setFont(fontObj.asFont());

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

    inline bool op_stringwidth(PSVirtualMachine &vm) {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();
        auto& ctm = grph->getCTM();

        if (ostk.size() < 1)
            return vm.error("op_stringwidth: stackunderflow;");
        
        PSObject fontObj, stringObj;
        if (!ostk.pop(stringObj) || !stringObj.isString())
            return vm.error("op_stringwidth: typecheck; expected string");
        
        auto fontHandle = vm.graphics()->currentFont(); // Ensure current font is set
        if (fontHandle == nullptr)
            return vm.error("op_stringwidth: no current font set");

        // ask the graphics sub-system to tell us the string width
        double dx = 0.0, dy = 0.0;
        grph->getStringWidth(fontHandle, stringObj.asString(), dx, dy);

        // Update the current path's current position, using non-transformed coordinates
        grph->currentPath().fCurrentX += dx;
        grph->currentPath().fCurrentY += dy;

        // transform by ctm, and return that
        ctm.dtransform(dx, dy, dx, dy);

        ostk.pushReal(dx);
        ostk.pushReal(dy);
        
        return true;
    }

    bool op_charpath(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();
        PSMatrix ctm = grph->getCTM();
        auto& currentPath = grph->currentPath();

        if (ostk.size() < 2)
            return vm.error("op_charpath: stackunderflow;");

        // Ensure current font is set
        auto fontHandle = grph->currentFont(); 
        if (fontHandle == nullptr)
            return vm.error("op_charpath: no current font set");

        PSObject posAsPrintedObj;
        PSObject strObj;

        ostk.pop(posAsPrintedObj);
        ostk.pop(strObj);

        if (!posAsPrintedObj.isBool())
            return vm.error("op_charpath: typecheck; expected boolean for position as printed");

        if (!strObj.isString())
            return vm.error("op_charpath: typecheck; expected string for charpath");


        bool success = grph->getCharPath(fontHandle, ctm, strObj.asString(), currentPath);

        return success;
    }

    //
    // BL_API BLResult BL_CDECL blFontGetGlyphOutlines(const BLFontCore* self, BLGlyphId glyphId, const BLMatrix2D* userTransform, BLPathCore* out, BLPathSinkFunc sink, void* userData) BL_NOEXCEPT_C;
    // BL_API BLResult BL_CDECL blFontGetGlyphRunOutlines(const BLFontCore* self, const BLGlyphRun* glyphRun, const BLMatrix2D* userTransform, BLPathCore* out, BLPathSinkFunc sink, void* userData) BL_NOEXCEPT_C;
    //
    // BL_INLINE_NODEBUG BLResult getGlyphOutlines(BLGlyphId glyphId, const BLMatrix2D& userTransform, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* userData = nullptr) const noexcept 
    // {
    //   return blFontGetGlyphOutlines(this, glyphId, &userTransform, &out, sink, userData);
    // }

    // BL_INLINE_NODEBUG BLResult getGlyphRunOutlines(const BLGlyphRun& glyphRun, BLPathCore& out, BLPathSinkFunc sink = nullptr, void* userData = nullptr) const noexcept 
    // {
    //   return blFontGetGlyphRunOutlines(this, &glyphRun, nullptr, &out, sink, userData);
    // }
    // 
    
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
            { "selectfont",   op_selectfont },
            { "stringwidth",  op_stringwidth},
            { "charpath",     op_charpath }
        };
        return table;
    }
}

