// ps_type_opentypefontresource.h
#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "blend2d_connect.h"
#include "pscore.h"
#include "ps_type_fontresource.h"
#include "pathprogram_builder.h"

namespace waavs
{
    struct PSOpenTypeGlyphEntry
    {
        mutable bool metricsReady{};
        mutable bool pathReady{};
        mutable PSGlyphMetrics metrics{};
        mutable PathProgram path;
    };

    struct PSGlyphNameUnicode
    {
        const char* name;
        uint32_t codepoint;
    };

    // Common Adobe/PostScript glyph names used by StandardEncoding,
    // ISOLatin1Encoding, WinAnsiEncoding, and ordinary Latin fonts.
    //
    // This table is intentionally a bridge from PostScript glyph-name semantics
    // to the Unicode cmap semantics used by most OpenType fonts. It is not tied
    // to any particular OpenType font.
    inline bool psGlyphNameToUnicode(const PSName& psName, uint32_t& codepoint) noexcept
    {
        const char* name = psName.c_str();
        if (!name || !name[0])
            return false;

        if (std::strcmp(name, ".notdef") == 0)
            return false;

        // Adobe glyph names may carry a suffix such as A.swash. Ignore the suffix
        // for cmap lookup.
        char baseName[64];
        size_t length = 0;
        while (name[length] && name[length] != '.' && length + 1 < sizeof(baseName)) {
            baseName[length] = name[length];
            ++length;
        }
        baseName[length] = '\0';

        if (length == 1) {
            unsigned char c = static_cast<unsigned char>(baseName[0]);
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                codepoint = c;
                return true;
            }
        }

        // uniXXXX is the common Adobe convention for a BMP code point.
        if (length == 7 && baseName[0] == 'u' && baseName[1] == 'n' && baseName[2] == 'i') {
            uint32_t value = 0;
            for (size_t i = 3; i < 7; ++i) {
                char c = baseName[i];
                uint32_t nibble;
                if (c >= '0' && c <= '9') nibble = static_cast<uint32_t>(c - '0');
                else if (c >= 'A' && c <= 'F') nibble = static_cast<uint32_t>(c - 'A' + 10);
                else if (c >= 'a' && c <= 'f') nibble = static_cast<uint32_t>(c - 'a' + 10);
                else return false;
                value = (value << 4) | nibble;
            }

            if (value >= 0xD800u && value <= 0xDFFFu)
                return false;

            codepoint = value;
            return true;
        }

        // uXXXX, uXXXXX, or uXXXXXX.
        if (length >= 5 && length <= 7 && baseName[0] == 'u') {
            uint32_t value = 0;
            for (size_t i = 1; i < length; ++i) {
                char c = baseName[i];
                uint32_t nibble;
                if (c >= '0' && c <= '9') nibble = static_cast<uint32_t>(c - '0');
                else if (c >= 'A' && c <= 'F') nibble = static_cast<uint32_t>(c - 'A' + 10);
                else if (c >= 'a' && c <= 'f') nibble = static_cast<uint32_t>(c - 'a' + 10);
                else return false;
                value = (value << 4) | nibble;
            }

            if (value > 0x10FFFFu || (value >= 0xD800u && value <= 0xDFFFu))
                return false;

            codepoint = value;
            return true;
        }

        static constexpr PSGlyphNameUnicode glyphNames[] = {
            {"space", 0x0020},
            {"exclam", 0x0021},
            {"quotedbl", 0x0022},
            {"numbersign", 0x0023},
            {"dollar", 0x0024},
            {"percent", 0x0025},
            {"ampersand", 0x0026},
            {"quotesingle", 0x0027},
            {"quoteright", 0x2019},
            {"parenleft", 0x0028},
            {"parenright", 0x0029},
            {"asterisk", 0x002A},
            {"plus", 0x002B},
            {"comma", 0x002C},
            {"hyphen", 0x002D},
            {"period", 0x002E},
            {"slash", 0x002F},
            {"zero", 0x0030},
            {"one", 0x0031},
            {"two", 0x0032},
            {"three", 0x0033},
            {"four", 0x0034},
            {"five", 0x0035},
            {"six", 0x0036},
            {"seven", 0x0037},
            {"eight", 0x0038},
            {"nine", 0x0039},
            {"colon", 0x003A},
            {"semicolon", 0x003B},
            {"less", 0x003C},
            {"equal", 0x003D},
            {"greater", 0x003E},
            {"question", 0x003F},
            {"at", 0x0040},
            {"bracketleft", 0x005B},
            {"backslash", 0x005C},
            {"bracketright", 0x005D},
            {"asciicircum", 0x005E},
            {"underscore", 0x005F},
            {"grave", 0x0060},
            {"quoteleft", 0x2018},
            {"braceleft", 0x007B},
            {"bar", 0x007C},
            {"braceright", 0x007D},
            {"asciitilde", 0x007E},

            {"exclamdown", 0x00A1},
            {"cent", 0x00A2},
            {"sterling", 0x00A3},
            {"currency", 0x00A4},
            {"yen", 0x00A5},
            {"brokenbar", 0x00A6},
            {"section", 0x00A7},
            {"dieresis", 0x00A8},
            {"copyright", 0x00A9},
            {"ordfeminine", 0x00AA},
            {"guillemotleft", 0x00AB},
            {"logicalnot", 0x00AC},
            {"registered", 0x00AE},
            {"macron", 0x00AF},
            {"degree", 0x00B0},
            {"plusminus", 0x00B1},
            {"twosuperior", 0x00B2},
            {"threesuperior", 0x00B3},
            {"acute", 0x00B4},
            {"mu", 0x00B5},
            {"paragraph", 0x00B6},
            {"periodcentered", 0x00B7},
            {"cedilla", 0x00B8},
            {"onesuperior", 0x00B9},
            {"ordmasculine", 0x00BA},
            {"guillemotright", 0x00BB},
            {"onequarter", 0x00BC},
            {"onehalf", 0x00BD},
            {"threequarters", 0x00BE},
            {"questiondown", 0x00BF},

            {"Agrave", 0x00C0},
            {"Aacute", 0x00C1},
            {"Acircumflex", 0x00C2},
            {"Atilde", 0x00C3},
            {"Adieresis", 0x00C4},
            {"Aring", 0x00C5},
            {"AE", 0x00C6},
            {"Ccedilla", 0x00C7},
            {"Egrave", 0x00C8},
            {"Eacute", 0x00C9},
            {"Ecircumflex", 0x00CA},
            {"Edieresis", 0x00CB},
            {"Igrave", 0x00CC},
            {"Iacute", 0x00CD},
            {"Icircumflex", 0x00CE},
            {"Idieresis", 0x00CF},
            {"Eth", 0x00D0},
            {"Ntilde", 0x00D1},
            {"Ograve", 0x00D2},
            {"Oacute", 0x00D3},
            {"Ocircumflex", 0x00D4},
            {"Otilde", 0x00D5},
            {"Odieresis", 0x00D6},
            {"multiply", 0x00D7},
            {"Oslash", 0x00D8},
            {"Ugrave", 0x00D9},
            {"Uacute", 0x00DA},
            {"Ucircumflex", 0x00DB},
            {"Udieresis", 0x00DC},
            {"Yacute", 0x00DD},
            {"Thorn", 0x00DE},
            {"germandbls", 0x00DF},
            {"agrave", 0x00E0},
            {"aacute", 0x00E1},
            {"acircumflex", 0x00E2},
            {"atilde", 0x00E3},
            {"adieresis", 0x00E4},
            {"aring", 0x00E5},
            {"ae", 0x00E6},
            {"ccedilla", 0x00E7},
            {"egrave", 0x00E8},
            {"eacute", 0x00E9},
            {"ecircumflex", 0x00EA},
            {"edieresis", 0x00EB},
            {"igrave", 0x00EC},
            {"iacute", 0x00ED},
            {"icircumflex", 0x00EE},
            {"idieresis", 0x00EF},
            {"eth", 0x00F0},
            {"ntilde", 0x00F1},
            {"ograve", 0x00F2},
            {"oacute", 0x00F3},
            {"ocircumflex", 0x00F4},
            {"otilde", 0x00F5},
            {"odieresis", 0x00F6},
            {"divide", 0x00F7},
            {"oslash", 0x00F8},
            {"ugrave", 0x00F9},
            {"uacute", 0x00FA},
            {"ucircumflex", 0x00FB},
            {"udieresis", 0x00FC},
            {"yacute", 0x00FD},
            {"thorn", 0x00FE},
            {"ydieresis", 0x00FF},

            {"OE", 0x0152},
            {"oe", 0x0153},
            {"Scaron", 0x0160},
            {"scaron", 0x0161},
            {"Ydieresis", 0x0178},
            {"Zcaron", 0x017D},
            {"zcaron", 0x017E},
            {"florin", 0x0192},
            {"circumflex", 0x02C6},
            {"caron", 0x02C7},
            {"breve", 0x02D8},
            {"dotaccent", 0x02D9},
            {"ring", 0x02DA},
            {"ogonek", 0x02DB},
            {"tilde", 0x02DC},
            {"hungarumlaut", 0x02DD},

            {"endash", 0x2013},
            {"emdash", 0x2014},
            {"quotesinglbase", 0x201A},
            {"quotedblleft", 0x201C},
            {"quotedblright", 0x201D},
            {"quotedblbase", 0x201E},
            {"dagger", 0x2020},
            {"daggerdbl", 0x2021},
            {"bullet", 0x2022},
            {"ellipsis", 0x2026},
            {"perthousand", 0x2030},
            {"guilsinglleft", 0x2039},
            {"guilsinglright", 0x203A},
            {"fraction", 0x2044},
            {"Euro", 0x20AC},
            {"trademark", 0x2122},
            {"minus", 0x2212},
            {"fi", 0xFB01},
            {"fl", 0xFB02}
        };

        for (const auto& entry : glyphNames) {
            if (std::strcmp(baseName, entry.name) == 0) {
                codepoint = entry.codepoint;
                return true;
            }
        }

        return false;
    }

    class PSOpenTypeFontResource final : public PSFontResource
    {
    public:
        PSFontType type() const noexcept override
        {
            return fType;
        }

        const PSMatrix& fontMatrix() const noexcept override
        {
            return fFontMatrix;
        }

        bool glyphForName(const PSName& name, PSGlyphId& glyphId) const override
        {
            if (!fFace.is_valid())
                return false;

            if (name == PSName(".notdef")) {
                if (fGlyphCount == 0)
                    return false;

                glyphId = 0;
                return true;
            }

            uint32_t codepoint;
            if (!psGlyphNameToUnicode(name, codepoint))
                return false;

            return glyphForUnicode(codepoint, glyphId);
        }

        bool glyphMetrics(PSGlyphId glyphId, PSGlyphMetrics& metrics) const override
        {
            if (!loadMetrics(glyphId))
                return false;

            metrics = fGlyphs[glyphId].metrics;
            return true;
        }

        PSGlyphKind glyphKind(PSGlyphId glyphId) const noexcept override
        {
            (void)glyphId;
            return PSGlyphKind::Outline;
        }

        const PathProgram* glyphPath(PSGlyphId glyphId) const override
        {
            if (!loadGlyphPath(glyphId))
                return nullptr;

            return &fGlyphs[glyphId].path;
        }

        bool glyphForUnicode(uint32_t codepoint, PSGlyphId& glyphId) const
        {
            if (!fFont.is_valid() || codepoint > 0x10FFFFu || (codepoint >= 0xD800u && codepoint <= 0xDFFFu))
                return false;

            BLGlyphBuffer gb;
            if (gb.set_utf32_text(&codepoint, 1) != BL_SUCCESS)
                return false;

            if (fFont.map_text_to_glyphs(gb) != BL_SUCCESS)
                return false;

            if (gb.has_undefined_chars() || gb.size() != 1)
                return false;

            uint32_t id = gb.content()[0];
            if (id == 0 || id >= fGlyphCount)
                return false;

            glyphId = static_cast<PSGlyphId>(id);
            return true;
        }

        const BLFontFace& face() const noexcept
        {
            return fFace;
        }

        uint32_t glyphCount() const noexcept
        {
            return fGlyphCount;
        }

        uint32_t unitsPerEm() const noexcept
        {
            return fUnitsPerEm;
        }

        const BLString& postScriptName() const noexcept
        {
            return fFace.post_script_name();
        }

        const BLString& familyName() const noexcept
        {
            return fFace.family_name();
        }

        const BLString& subfamilyName() const noexcept
        {
            return fFace.subfamily_name();
        }

        static std::shared_ptr<PSOpenTypeFontResource> create(const BLFontFace& face)
        {
            if (!face.is_valid())
                return {};

            auto resource = std::make_shared<PSOpenTypeFontResource>();
            if (!resource->hydrate(face))
                return {};

            return resource;
        }

        static std::shared_ptr<PSOpenTypeFontResource> create(const char* filePath)
        {
            if (!filePath || !filePath[0])
                return {};

            BLFontFace face;
            if (face.create_from_file(filePath) != BL_SUCCESS || !face.is_valid())
                return {};

            return create(face);
        }

    private:
        bool hydrate(const BLFontFace& face)
        {
            fFace = face;

            const BLFontDesignMetrics& dm = fFace.design_metrics();
            if (dm.units_per_em <= 0 || fFace.glyph_count() == 0)
                return false;

            fUnitsPerEm = static_cast<uint32_t>(dm.units_per_em);
            fGlyphCount = fFace.glyph_count();
            fFontMatrix = PSMatrix(1.0 / static_cast<double>(fUnitsPerEm), 0.0, 0.0,
                                   1.0 / static_cast<double>(fUnitsPerEm), 0.0, 0.0);

            switch (fFace.outline_type()) {
            case BL_FONT_OUTLINE_TYPE_CFF:
                fType = PSFontType::CFF;
                break;
            case BL_FONT_OUTLINE_TYPE_CFF2:
                fType = PSFontType::CFF2;
                break;
            case BL_FONT_OUTLINE_TYPE_TRUETYPE:
                fType = PSFontType::OpenType;
                break;
            default:
                return false;
            }

            // Create a Blend2D font at exactly unitsPerEm. That makes Blend2D's
            // scaled glyph coordinates numerically equal to design units, which
            // is the native coordinate convention expected by PSFontResource.
            if (fFont.create_from_face(fFace, static_cast<float>(fUnitsPerEm)) != BL_SUCCESS || !fFont.is_valid())
                return false;

            fGlyphs.clear();
            fGlyphs.resize(fGlyphCount);
            return true;
        }

        bool loadMetrics(PSGlyphId glyphId) const
        {
            if (glyphId >= fGlyphCount)
                return false;

            PSOpenTypeGlyphEntry& glyph = fGlyphs[glyphId];
            if (glyph.metricsReady)
                return true;

            uint32_t id = static_cast<uint32_t>(glyphId);
            BLGlyphBuffer gb;
            if (gb.set_glyphs(&id, 1) != BL_SUCCESS)
                return false;

            if (fFont.position_glyphs(gb) != BL_SUCCESS)
                return false;

            BLTextMetrics tm;
            if (fFont.get_text_metrics(gb, tm) != BL_SUCCESS)
                return false;

            glyph.metrics.advanceX = tm.advance.x;
            glyph.metrics.advanceY = tm.advance.y;
            glyph.metrics.sideBearingX = tm.leading_bearing.x;
            glyph.metrics.sideBearingY = tm.leading_bearing.y;
            glyph.metrics.bboxXMin = tm.bounding_box.x0;
            glyph.metrics.bboxYMin = tm.bounding_box.y0;
            glyph.metrics.bboxXMax = tm.bounding_box.x1;
            glyph.metrics.bboxYMax = tm.bounding_box.y1;
            glyph.metrics.hasBBox = true;
            glyph.metricsReady = true;

            return true;
        }

        bool loadGlyphPath(PSGlyphId glyphId) const
        {
            if (glyphId >= fGlyphCount)
                return false;

            PSOpenTypeGlyphEntry& glyph = fGlyphs[glyphId];
            if (glyph.pathReady)
                return true;

            BLPath blPath;
            if (fFont.get_glyph_outlines(static_cast<BLGlyphId>(glyphId), blPath) != BL_SUCCESS)
                return false;

            PSMatrix glyphNormalMatrix;
            glyphNormalMatrix.scale(1.0, -1.0);

            PathProgram prog;
            if (!convertBLPathToPathProgram(blPath, prog, glyphNormalMatrix))
                return false;

            glyph.path = std::move(prog);
            glyph.pathReady = true;
            return true;
        }



        PSFontType fType{ PSFontType::Unknown };
        BLFontFace fFace;
        BLFont fFont;
        PSMatrix fFontMatrix{};
        uint32_t fGlyphCount{};
        uint32_t fUnitsPerEm{};
        mutable std::vector<PSOpenTypeGlyphEntry> fGlyphs;
    };

    INLINE const char* toLowerIntern(const BLString& str) {
        std::string temp(static_cast<const char*>(str.data()), str.size());
        for (auto& c : temp) c = (char)std::tolower(c);
        return PSNameTable::INTERN(temp.c_str(), temp.size());
    }


    bool loadFontResource(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("loadFontResource: stackunderflow");

        PSObject pathObj;
        if (!s.pop(pathObj))
            return vm.error("loadFontResource: stackunderflow");

        if (!pathObj.isString())
            return vm.error("loadFontResource: typecheck; expected font path string");

        auto filePathStr = pathObj.asString().toString();
        const char* filePath = filePathStr.c_str();

        BLFontFace face;
        if (face.create_from_file(filePath) != BL_SUCCESS || !face.is_valid())
            return vm.error("loadFontResource: failed to load OpenType font");

        auto resource = PSOpenTypeFontResource::create(face);
        if (!resource)
            return vm.error("loadFontResource: failed to create OpenType font resource");

        const BLFontDesignMetrics& dm = face.design_metrics();

        const char* psName = toLowerIntern(face.post_script_name());
        const char* famName = toLowerIntern(face.family_name());
        const char* subfamName = toLowerIntern(face.subfamily_name());

        if (!psName || !psName[0])
            return vm.error("loadFontResource: font has no PostScript name");

        auto fontDict = PSDictionary::create();

        fontDict->put("FontName", PSObject::fromName(PSName(psName)));
        fontDict->put("FamilyName", PSObject::fromName(PSName(famName)));
        fontDict->put("SubfamilyName", PSObject::fromName(PSName(subfamName)));

        fontDict->put("Weight", PSObject::fromInt(face.weight()));
        fontDict->put("Stretch", PSObject::fromInt(face.stretch()));
        fontDict->put("Style", PSObject::fromInt(face.style()));

        fontDict->put("UnitsPerEm", PSObject::fromInt(static_cast<int32_t>(resource->unitsPerEm())));
        fontDict->put("FontMatrix", PSObject::fromMatrix(resource->fontMatrix()));

        auto bbox = PSArray::create();
        bbox->append(PSObject::fromReal(dm.glyph_bounding_box.x0));
        bbox->append(PSObject::fromReal(dm.glyph_bounding_box.y0));
        bbox->append(PSObject::fromReal(dm.glyph_bounding_box.x1));
        bbox->append(PSObject::fromReal(dm.glyph_bounding_box.y1));
        fontDict->put("FontBBox", PSObject::fromArray(bbox));

        PSObject encodingObj;
        if (vm.dictionaryStack.load("StandardEncoding", encodingObj) && encodingObj.isArray())
            fontDict->put("Encoding", encodingObj);

        auto font = PSFont::create(resource, fontDict);
        if (!font)
            return vm.error("loadFontResource: failed to create font object");

        PSObject fontObj = PSObject::fromDictionary(font);

        if (!vm.defineResource(PSName(psName), PSName("Font"), fontObj))
            return vm.error("loadFontResource: failed to define font resource", psName);

        return true;
    }



} // namespace waavs
