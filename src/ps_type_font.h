#pragma once

#include <memory>

#include "pscore.h"  // Provides PSObject, PSMatrix, PSDictionaryHandle, etc.
#include "ps_type_dictionary.h"

namespace waavs {
    struct PSFontFace;
    struct PSFontFace;





    // ------------------------------------------------------
    // PSFontFace - 
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
    struct PSFontFace {
    private:
        // This is what the system uses to identify the fontface
        // The system that is loading font information will put something here
        // that associates font face information for later building of scaled
        // fonts.  It's opaque to the PostScript code, so we remain backend agnostic
        void* fSystemHandle{ nullptr };      
        PSDictionaryHandle fDict;           // Dictionary of font properties

    public:
        PSFontFace() : fDict(PSDictionary::create()) 
        {
            
        }

        void setSystemHandle(void* handle) {fSystemHandle = handle;}
        void* getSystemHandle() const {return fSystemHandle;}

        PSDictionaryHandle getDictionary() const { return fDict; }

        void set(const PSName & key, const PSObject& value) 
        {
            fDict->put(key, value);
        }

        bool get(const PSName & key, PSObject& out) const {
            return fDict->get(key, out);
        }

        bool contains(const PSName & key) const {
            return fDict->contains(key);
        }

        static PSFontFaceHandle create() {
            auto face = std::make_shared<PSFontFace>();

            return face;
        }
    };


    // ------------------------------------------------------------------
// PSFont - Represents a fully sized and transformed font instance
// 
// Standard keys for a font instance include:
// Red Book 3.0, Section 5.2.2 "Font Instance Dictionary"
// 
// Fields typically copied through from the font face:
// FontName
// FontType
// FontBBox
// FontMatrix
// Encoding
// CharStrings
// BuildChar
// BuildGlyph
// 
// FID              Unique key assigned by PostScript interpreter
// FontMatrix       Transformed version of the FontMatrix (after scaling)
// FontInfo         Optional dictionary with additional font info
// WMode            Writing mode (0 for horizontal, 1 for vertical)
// PaintType        (Type 3 fonts only)
// StrokeWidth      (Type 3 or stroked fonts)
// UniqueID         (copied if present)
// FMapType         For composite fonts - describes font mapping strategy
// Metrics          For CID Fonts, (horizontal and vertical) metrics
// Metrics2
// ------------------------------------------------------------------
    struct PSFont {
        void* fSystemHandle{ nullptr }; // System-specific handle for the font instance
        PSDictionaryHandle fDict;       // Dictionary exposed as /currentfont

        PSMatrix transform;            // FontMatrix after scaling or transform




        PSFont() : fDict(PSDictionary::create()) {}

        PSFont(void *sysHandle) 
            : fSystemHandle(sysHandle), fDict(PSDictionary::create()) 
        {
            // Initialize with a system handle, if provided
        }


        PSDictionaryHandle getDictionary() const { return fDict; }
        void* getSystemHandle() const { return fSystemHandle; }
        constexpr void setSystemHandle(void* handle) { fSystemHandle = handle; }


        void put(const PSName& key, const PSObject& value)
        {
            fDict->put(key, value);
        }

        bool get(const PSName& key, PSObject& out) const 
        {
            return fDict->get(key, out);
        }

        static std::shared_ptr<PSFont> create(void* sysHandle)
        {
            return std::shared_ptr<PSFont>(new PSFont(sysHandle));
        }

    };
} // namespace waavs
