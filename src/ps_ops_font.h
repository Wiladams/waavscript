// ps_ops_font.h
#pragma once

#include "pscore.h"
#include "psvm.h"
#include "ps_object_helpers.h"
#include "ps_type_font.h"
#include "ps_type_type1fontresource.h"

namespace waavs {

    inline bool op_findfont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject nameObj;
        if (!s.pop(nameObj))
            return vm.error("findfont: stackunderflow");

        if (nameObj.isString()) {
            const PSString& str = nameObj.asString();
            nameObj = PSObject::fromName(PSName(reinterpret_cast<const char*>(str.data()), str.length()));
        }

        if (!nameObj.isName())
            return vm.error("findfont: typecheck");

        // DIAGNOSTIC
        {
            printf("findfont: ");
            writeObjectDeep(nameObj);
            printf("\n");
        }

        PSName fontName = nameObj.asName();

        // Honor FontMap aliases if present.
        PSObject fontMapObj;
        if (vm.dictionaryStack.load("FontMap", fontMapObj) && fontMapObj.isDictionary()) {
            PSObject mappedObj;
            if (fontMapObj.asDictionary()->get(fontName, mappedObj) && mappedObj.isName())
                fontName = mappedObj.asName();
        }

        PSObject fontObj;
        if (!vm.findResource(fontName, PSName("Font"), fontObj))
            return vm.error("findfont: undefined");

        if (!fontObj.isDictionary())
            return vm.error("findfont: typecheck, resource is not a dictionary");

        return s.push(fontObj);
    }


    inline bool op_scalefont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject scaleObj;
        PSObject fontObj;

        if (!s.pop(scaleObj))
            return vm.error("scalefont: stackunderflow");

        if (!scaleObj.isNumber())
            return vm.error("scalefont: typecheck");

        if (!s.pop(fontObj))
            return vm.error("scalefont: stackunderflow");

        PSFontHandle source = getNativeFont(fontObj);

        if (!source || !source->valid())
            return vm.error("scalefont: invalidfont");

        const double scale = scaleObj.asReal();

        PSMatrix matrix = source->matrix();
        matrix.scale(scale, scale);

        PSFontHandle font = PSFont::create(source->resource, source, matrix);
        return s.push(PSObject::fromDictionary(std::move(font)));
    }


    inline bool op_makefont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject matrixObj;
        PSObject fontObj;

        if (!s.pop(matrixObj))
            return vm.error("makefont: stackunderflow");

        PSMatrix transform;
        if (!extractMatrix(matrixObj, transform))
            return vm.error("makefont: typecheck");

        if (!s.pop(fontObj))
            return vm.error("makefont: stackunderflow");

        PSFontHandle fontHand = getNativeFont(fontObj);

        if (!fontHand || !fontHand->valid())
            return vm.error("makefont: invalid font object");

        PSMatrix matrix = fontHand->matrix();
        matrix.preMultiply(transform);

        PSFontHandle newFont = PSFont::create(fontHand->resource, fontHand,  matrix);
        return s.push(PSObject::fromDictionary(std::move(newFont )));
    }


    inline bool op_setfont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject fontObj;
        if (!s.pop(fontObj))
            return vm.error("setfont: stackunderflow");

        PSFontHandle font = getNativeFont(fontObj);

        if (!font || !font->valid())
            return vm.error("setfont: invalidfont");

        vm.graphics()->setFont(font);

        return true;
    }


    inline bool op_currentfont(PSVirtualMachine& vm)
    {
        PSFontHandle font = vm.graphics()->currentFont();

        if (!font || !font->valid())
            return vm.error("currentfont: invalidfont");

        return vm.opStack().push(PSObject::fromDictionary(font));
    }


    inline bool op_definefont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject fontDictObj;
        PSObject nameObj;

        if (!s.pop(fontDictObj))
            return vm.error("definefont: stackunderflow");

        if (!fontDictObj.isDictionary())
            return vm.error("definefont: typecheck (fontDictObj)");

        if (!s.pop(nameObj))
            return vm.error("definefont: stackunderflow");

        if (!nameObj.isName())
            return vm.error("definefont: typecheck (nameObj)");

        PSDictionaryHandle fontDict = fontDictObj.asDictionary();

        PSObject fontTypeObj;
        if (!fontDict->get(PSName("FontType"), fontTypeObj) || !fontTypeObj.isIntegralNumber())
            return vm.error("definefont: invalidfont");

        std::shared_ptr<PSFontResource> resource;

        switch (fontTypeObj.asInt()) {
        case 1:
            resource = PSType1FontResource::create(fontDict);
            break;

        default:
            return vm.error("definefont: invalidfont");
        }

        if (!resource)
            return vm.error("definefont: invalidfont");

        auto newFont = PSFont::create(std::move(resource), fontDict);
        PSObject newFontObj = PSObject::fromDictionary(newFont);

        if (!vm.defineResource(nameObj.asName(), PSName("Font"), newFontObj))
            return vm.error("definefont: invalidfont");

        return s.push(newFontObj);
    }


    inline bool op_selectfont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject scaleObj;
        PSObject nameObj;

        if (!s.pop(scaleObj))
            return vm.error("selectfont: stackunderflow");

        if (!s.pop(nameObj))
            return vm.error("selectfont: stackunderflow");

        if (!nameObj.isName() && !nameObj.isString())
            return vm.error("selectfont: typecheck");

        if (!scaleObj.isNumber() && !scaleObj.isArray() && !scaleObj.isMatrix())
            return vm.error("selectfont: typecheck");

        if (!s.push(nameObj))
            return false;

        if (!op_findfont(vm))
            return false;

        if (!s.push(scaleObj))
            return false;

        if (scaleObj.isNumber()) {
            if (!op_scalefont(vm))
                return false;
        }
        else {
            if (!op_makefont(vm))
                return false;
        }

        return op_setfont(vm);
    }


    inline bool op_undefinefont(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject nameObj;
        if (!s.pop(nameObj))
            return vm.error("undefinefont: stackunderflow");

        if (!nameObj.isName())
            return vm.error("undefinefont: typecheck");

        // Wire this to the general resource removal API when available.
        return vm.error("undefinefont not implemented");
    }


    inline bool op_stringwidth(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto* grph = vm.graphics();

        if (s.empty())
            return vm.error("op_stringwidth: stackunderflow");

        PSObject stringObj;
        if (!s.pop(stringObj) || !stringObj.isString())
            return vm.error("op_stringwidth: typecheck; expected string");

        if (!grph)
            return vm.error("op_stringwidth: no graphics context");

        PSFontHandle font = grph->currentFont();
        if (!font || !font->valid())
            return vm.error("op_stringwidth: no current font");

        const PSString& str = stringObj.asString();
        const PSMatrix fontMatrix = font->matrix();

        double widthX = 0.0;
        double widthY = 0.0;

        for (size_t i = 0; i < str.length(); ++i)
        {
            PSResolvedGlyph glyph;

            if (!grph->resolveGlyph(str.data()[i], glyph))
                return vm.error("op_stringwidth: unable to resolve glyph");

            double dx = glyph.metrics.advanceX;
            double dy = glyph.metrics.advanceY;

            fontMatrix.dtransform(dx, dy, dx, dy);

            widthX += dx;
            widthY += dy;
        }

        s.pushReal(widthX);
        s.pushReal(widthY);

        return true;
    }


    inline bool op_charpath(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto* grph = vm.graphics();

        if (s.size() < 2)
            return vm.error("op_charpath: stackunderflow");

        PSObject strokeObj;
        PSObject stringObj;

        if (!s.pop(strokeObj) || !s.pop(stringObj))
            return vm.error("op_charpath: stackunderflow");

        if (!stringObj.isString())
            return vm.error("op_charpath: typecheck; expected string");

        if (!strokeObj.isBool())
            return vm.error("op_charpath: typecheck; expected boolean");

        if (!grph)
            return vm.error("op_charpath: no graphics context");

        PSFontHandle font = grph->currentFont();
        if (!font || !font->valid())
            return vm.error("op_charpath: no current font");

        const PSString& str = stringObj.asString();
        const PSMatrix& ctm = grph->getCTM();
        const PSMatrix fontMatrix = font->matrix();
        PSPath& currentPath = grph->currentPath();

        double x;
        double y;

        if (!currentPath.getCurrentPoint(ctm, x, y))
            return vm.error("op_charpath: nocurrentpoint");

        // For ordinary outline fonts the boolean does not change the glyph
        // outline geometry we append here. Keep it available for font types
        // whose charpath realization may eventually depend on it.
        const bool stroke = strokeObj.asBool();
        (void)stroke;

        for (size_t i = 0; i < str.length(); ++i)
        {
            PSResolvedGlyph glyph;

            if (!grph->resolveGlyph(str.data()[i], glyph))
                return vm.error("op_charpath: unable to resolve glyph");

            if (font->resource->glyphKind(glyph.glyphId) != PSGlyphKind::Outline)
                return vm.error("op_charpath: non-outline glyph");

            if (!glyph.path)
                return vm.error("op_charpath: missing glyph path");

            PSMatrix transform = ctm;
            transform.translate(x, y);
            transform.preMultiply(fontMatrix);

            if (!currentPath.appendPath(*glyph.path, transform))
                return vm.error("op_charpath: appendPath failed");

            double dx = glyph.metrics.advanceX;
            double dy = glyph.metrics.advanceY;

            fontMatrix.dtransform(dx, dy, dx, dy);

            x += dx;
            y += dy;
        }

        if (!currentPath.setCurrentPoint(ctm, x, y))
            return vm.error("op_charpath: failed to update current point");

        return true;
    }


    inline const PSOperatorFuncMap& getFontOps()
    {
        static const PSOperatorFuncMap table = {
            { "findfont",     op_findfont },
            { "scalefont",    op_scalefont },
            { "makefont",     op_makefont },
            { "setfont",      op_setfont },
            { "currentfont",  op_currentfont },
            { "definefont",   op_definefont },
            { "undefinefont", op_undefinefont },
            { "selectfont",   op_selectfont },
            { "stringwidth",  op_stringwidth },
            { "charpath",     op_charpath }
        };

        return table;
    }

} // namespace waavs