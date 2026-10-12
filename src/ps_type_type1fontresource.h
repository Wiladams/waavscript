// ps_type_type1fontresource.h
#pragma once

#include <memory>
#include <vector>

#include "pscore.h"
#include "ps_type_fontresource.h"
#include "ps_type_dictionary.h"

#include "type1_charstring_decoder.h"

namespace waavs {

    struct PSType1GlyphEntry
    {
        PSName name;
        PSString charString;

        mutable bool decoded{};
        mutable PSGlyphMetrics metrics{};
        mutable PathProgram path;
    };

    // ------------------------------------------------------
    // PSType1FontResource - 
    // 
    // Represents a named, unsized font face.  Once this object is created,
    // it can be used to create sized font objects (PSFont).
    // 
    // There is a standard set of key names used for a font face:
    // Red Book 3.0, Section 5.2.1, or 5.4.2 "Font Dictionary"
    // 
    // FontName         PostScript name of the font face (a literal name object)
    // FontType         Integer indicating the type of font (1 for Type 1, 2 for TrueType, etc.)
    // FontMatrix       6-element array (matrix) mapping glyph space to user space
    // FontBBox         4-element array specifying bounding box in glyph space [llx, lly, urx, ury]
    // Encoding         Array of 256 names (usually glyph names), indexed by character code
    // CharStrings      Dictionary mapping glyph names to charstring definitions (Type 1)
    // PaintType        For Type 3 fonts, specifies paint type (0 for fill, 1 for stroke)
    // Private          Dictionary for font-specific data (Type 1 and Type 3 fonts)
    // BuildChar        Procedure for building glyphs on demand (Type 3 fonts)
    // BuildGlyph       Alternative procedure for building glyphs (Type 3 fonts)
    // ------------------------------------------------------
    class PSType1FontResource final : public PSFontResource
    {
    public:
        PSFontType type() const noexcept override { return PSFontType::Type1; }
        const PSMatrix& fontMatrix() const noexcept override { return fFontMatrix; }


        bool glyphForName(const PSName& name, PSGlyphId& glyphId) const override
        {
            for (size_t i = 0; i < fGlyphs.size(); ++i) {
                if (fGlyphs[i].name == name) {
                    glyphId = static_cast<PSGlyphId>(i);
                    return true;
                }
            }

            return false;
        }

        bool glyphMetrics(PSGlyphId glyphId, PSGlyphMetrics& metrics) const override
        {
            if (!decodeGlyph(glyphId))
                return false;

            metrics = fGlyphs[glyphId].metrics;
            return true;
        }

        PSGlyphKind glyphKind(PSGlyphId glyphId) const noexcept override
        {
            return PSGlyphKind::Outline;
        }

        const PathProgram* glyphPath(PSGlyphId glyphId) const override
        {
            if (!decodeGlyph(glyphId))
                return nullptr;

            return &fGlyphs[glyphId].path;
        }


        static std::shared_ptr<PSType1FontResource> create(const PSDictionaryHandle& fontDict)
        {
            if (!fontDict)
                return {};

            auto resource = std::make_shared<PSType1FontResource>();
            if (!resource->hydrate(fontDict))
                return {};

            return resource;
        }

    private:


        bool hydrate(const PSDictionaryHandle& fontDict)
        {
            PSObject obj;

            if (!fontDict->get(PSName("FontMatrix"), obj))
                return false;

            if (!extractMatrix(obj, fFontMatrix))
                return false;

            if (!fontDict->get(PSName("Private"), obj) || !obj.isDictionary())
                return false;

            if (!hydratePrivate(obj.asDictionary()))
                return false;

            if (!fontDict->get(PSName("CharStrings"), obj) || !obj.isDictionary())
                return false;

            if (!hydrateCharStrings(obj.asDictionary()))
                return false;

            return true;
        }

        bool hydratePrivate(const PSDictionaryHandle& privateDict)
        {
            PSObject obj;

            // Type 1 default.
            fLenIV = 4;

            if (privateDict->get(PSName("lenIV"), obj)) {
                if (!obj.isIntegralNumber())
                    return false;

                fLenIV = obj.asInt();
            }

            if (privateDict->get(PSName("Subrs"), obj)) {
                if (!obj.isArray())
                    return false;

                if (!hydrateSubrs(obj.asArray()))
                    return false;
            }

            return true;
        }

        bool hydrateSubrs(const PSArrayHandle& subrs)
        {
            if (!subrs)
                return false;

            fSubrStrings.clear();
            fSubrs.clear();

            fSubrStrings.resize(subrs->size());
            fSubrs.resize(subrs->size());

            for (size_t i = 0; i < subrs->size(); ++i) {
                PSObject obj;

                if (!subrs->get(i, obj))
                    return false;

                // Preserve the original array index. A null slot stays empty.
                if (obj.isNull())
                    continue;

                if (!obj.isString())
                    return false;

                fSubrStrings[i] = obj.asString();

                const PSString& str = fSubrStrings[i];
                fSubrs[i] = OctetCursor(str.data(), str.length());
            }

            return true;
        }


        bool hydrateCharStrings(const PSDictionaryHandle& charStrings)
        {
            if (!charStrings)
                return false;

            fGlyphs.clear();
            fGlyphs.reserve(charStrings->size());
            fNotdefGlyph = kInvalidPSGlyphId;

            size_t cursor = 0;
            PSName glyphName;
            PSObject obj;

            while (charStrings->nextEntry(cursor, glyphName, obj)) {
                if (!obj.isString())
                    return false;

                const PSGlyphId glyphId = static_cast<PSGlyphId>(fGlyphs.size());

                PSType1GlyphEntry glyph;
                glyph.name = glyphName;
                glyph.charString = obj.asString();

                fGlyphs.push_back(std::move(glyph));

                if (glyphName == PSName(".notdef"))
                    fNotdefGlyph = glyphId;
            }

            return !fGlyphs.empty();
        }

        bool decodeGlyph(PSGlyphId glyphId) const
        {
            if (glyphId >= fGlyphs.size())
                return false;

            PSType1GlyphEntry& glyph = fGlyphs[glyphId];

            if (glyph.decoded)
                return true;

            OctetCursor charString(glyph.charString.data(), glyph.charString.length());

            PathProgramBuilder builder;
            Type1CharStringDecoder decoder(builder, fSubrs, fLenIV);

            Type1GlyphMetrics metrics;
            if (!decoder.decode(charString, metrics))
                return false;

            glyph.path = std::move(builder.prog);

            glyph.metrics.sideBearingX = metrics.sideBearingX;
            glyph.metrics.sideBearingY = metrics.sideBearingY;
            glyph.metrics.advanceX = metrics.advanceX;
            glyph.metrics.advanceY = metrics.advanceY;

            glyph.decoded = true;
            return true;
        }


        // ----------------------------------
        PSMatrix fFontMatrix{};
        int fLenIV{ 4 };
        PSGlyphId fNotdefGlyph{ kInvalidPSGlyphId };

        std::vector<PSString> fSubrStrings;
        std::vector<OctetCursor> fSubrs;
        mutable std::vector<PSType1GlyphEntry> fGlyphs;
    };

} // namespace waavs