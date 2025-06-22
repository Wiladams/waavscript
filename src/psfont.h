#pragma once

#include "pscore.h"  // Provides PSObject, PSMatrix, PSDictionaryHandle, etc.
#include "psdictionary.h"

namespace waavs {

    // ------------------------------------------------------
    // PSFontFace - Represents a named, unsized font face
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
        void* fSystemKey = nullptr;     // This is what the system uses to identify the fontface
        PSDictionaryHandle fDict;       // Dictionary of font properties

        PSFontFace() : fDict(PSDictionary::create()) 
        {
            
        }

        void set(const char* key, const PSObject& value) 
        {
            fDict->put(key, value);
        }

        bool get(const char* key, PSObject& out) const {
            return fDict->get(key, out);
        }

        static PSFontFaceHandle create(const PSString& name) {
            auto face = std::make_shared<PSFontFace>();

            face->set("FontName", PSObject::fromString(name));
            face->set("FontMatrix", PSObject::fromMatrix(PSMatrix()));

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
        //PSFontFaceHandle face;         // Back-reference to font face
        PSMatrix transform;            // FontMatrix after scaling or transform

        // Optional metrics
        double ascent = 0.0;
        double descent = 0.0;
        double bbox[4] = { 0.0, 0.0, 0.0, 0.0 };

        PSDictionaryHandle fDict;       // Dictionary exposed as /currentfont

        PSFont() : fDict(PSDictionary::create()) {}
        PSFont(const PSFontFaceHandle&face, double size) : fDict(PSDictionary::create())
        {
            if (!face) return; // Ensure face is valid

            fDict->put("FontFace", PSObject::fromFontFace(face)); // Store the face reference
            fDict->copyEntryFrom(*face->fDict.get(), "FontName"); // Copy the font name from the face
            fDict->put("PointSize", PSObject::fromReal(size)); // Store the point size

            // scale face matrix by size
            PSObject faceMatrixObj;
            PSMatrix fontMatrix = PSMatrix(); // Default to identity matrix

            if (face->get("FontMatrix", faceMatrixObj)) {
                fontMatrix = faceMatrixObj.asMatrix();
            }
            fontMatrix.scale(size, size); // Scale the matrix by the point size

            fDict->put("FontMatrix", PSObject::fromMatrix(fontMatrix));
        }

        void set(const char* key, const PSObject& value) 
        {
            fDict->put(key, value);
        }

        bool get(const char* key, PSObject& out) const {
            return fDict->get(key, out);
        }

        static PSFontHandle createFromSize(const PSFontFaceHandle& face, double size) {
            if (!face) return nullptr;

            auto font = std::shared_ptr<PSFont>(new PSFont(face, size));

            return font;
        }

        /*
        static PSFontHandle createFromMatrix(const PSFontFaceHandle& face, const PSMatrix& m) {
            if (!face) return nullptr;

            auto font = std::make_shared<PSFont>();
            font->set("FontFace", PSObject::fromFontFace(face)); // Store the face reference

            font->transform = m;

            font->set("FontName", PSObject::fromString(face->fName));
            font->set("FontMatrix", PSObject::fromMatrix(m));

            return font;
        }
        */
    };

} // namespace waavs
