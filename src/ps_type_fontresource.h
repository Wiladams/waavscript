// ps_type_fontresource.h
#pragma once

#include <cstdint>
#include <memory>

#include "ps_type_matrix.h"

namespace waavs
{

    class PathProgram;
    class PSName;

    // Opaque glyph identifier within a single font resource.
    // The meaning is resource-specific:
    //   - OpenType: typically the native glyph index.
    //   - Type 1: typically an internal CharStrings entry index.
    //   - Type 3: typically an internal BuildGlyph/BuildChar entry index.
    using PSGlyphId = uint32_t;

    inline constexpr PSGlyphId kInvalidPSGlyphId = UINT32_MAX;

    // Runtime font-program family.
    //
    // These values describe the resource implementation, not the PostScript
    // FontType dictionary value verbatim. Additional formats can be added without
    // changing the generic font consumer.
    enum class PSFontType : uint8_t
    {
        Unknown = 0,
        Type1,
        Type3,
        OpenType,
        CFF,
        CFF2,
        CID
    };

    // A glyph may either normalize to outline geometry or require execution.
    //
    // Outline glyphs are represented by PathProgram. Procedure glyphs are used by
    // Type 3 fonts, where the glyph is an executable PostScript procedure rather
    // than ordinary outline data.
    enum class PSGlyphKind : uint8_t
    {
        Outline,
        Procedure
    };

    // Font-space glyph metrics.
    //
    // The values are expressed in the resource's native glyph coordinate system.
    // FontMatrix and the selected font instance matrix are applied by the text
    // machinery when the glyph is measured or rendered.
    struct PSGlyphMetrics
    {
        double advanceX{};
        double advanceY{};
        double sideBearingX{};
        double sideBearingY{};

        double bboxXMin{};
        double bboxYMin{};
        double bboxXMax{};
        double bboxYMax{};
        bool hasBBox{};
    };

    // Immutable font-program/resource interface.
    //
    // A resource owns or refers to the underlying font program and hides its
    // format-specific representation. Glyph decoding may be lazy and resources
    // may cache compiled PathPrograms internally.
    //
    // Character interpretation and PostScript Encoding are intentionally not part
    // of this interface. They belong to the PostScript font object that refers to
    // the resource.
    class PSFontResource
    {
    public:
        virtual ~PSFontResource() = default;

        PSFontResource(const PSFontResource&) = delete;
        PSFontResource& operator=(const PSFontResource&) = delete;

        virtual PSFontType type() const noexcept = 0;

        // Intrinsic font-program matrix. For Type 1 and Type 3 this corresponds to
        // FontMatrix. Other resources expose the matrix that converts their native
        // glyph coordinate system into PostScript font space.
        virtual const PSMatrix& fontMatrix() const noexcept = 0;

        // Resolve a PostScript glyph name to an opaque resource-local glyph ID.
        //
        // This is the primary lookup used by ordinary PostScript Encoding arrays.
        // Other lookup mechanisms such as Unicode, native glyph index, CID, or CMap
        // resolution can be added as separate operations without changing this one.
        virtual bool glyphForName(const PSName& name, PSGlyphId& glyphId) const = 0;

        // Retrieve font-space metrics for a glyph.
        virtual bool glyphMetrics(PSGlyphId glyphId, PSGlyphMetrics& metrics) const = 0;

        // Describe how this glyph must be realized.
        virtual PSGlyphKind glyphKind(PSGlyphId glyphId) const noexcept = 0;

        // Return normalized outline geometry for an outline glyph.
        //
        // The returned PathProgram is owned by the resource and must remain valid
        // for at least as long as the resource. Implementations may compile and
        // cache the PathProgram lazily on first request.
        //
        // Returns nullptr when the glyph is not an outline glyph or cannot be
        // realized as an outline.
        virtual const PathProgram* glyphPath(PSGlyphId glyphId) const = 0;

    protected:
        PSFontResource() = default;
    };

} // namespace waavs
