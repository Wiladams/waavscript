#pragma once

#include "pscore.h"  // Provides PSObject, PSMatrix, PSDictionaryHandle, etc.

namespace waavs {

    // ------------------------------------------------------
    // PSFontFace - Represents a named, unsized font face
    // ------------------------------------------------------
    struct PSFontFace {
        char* fSystemKey = nullptr;     // This is what the system uses to identify the fontface
        PSString fName;                 // /FontName, e.g., "Helvetica"
        PSMatrix fDefaultMatrix;        // /FontMatrix, often identity or scaled
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
            face->fName = name;
            face->set("FontName", PSObject::fromString(name));
            face->set("FontMatrix", PSObject::fromMatrix(face->fDefaultMatrix));
            return face;
        }
    };

    // ------------------------------------------------------------------
    // PSFont - Represents a fully sized and transformed font instance
    // ------------------------------------------------------------------
    struct PSFont {
        PSFontFaceHandle face;         // Back-reference to font face
        PSMatrix transform;            // FontMatrix after scaling or transform
        double pointSize = 0.0;        // Size in points (for scalefont use)

        // Optional metrics
        double ascent = 0.0;
        double descent = 0.0;
        double bbox[4] = { 0.0, 0.0, 0.0, 0.0 };

        PSDictionaryHandle dict;       // Dictionary exposed as /currentfont

        PSFont() : dict(PSDictionary::create()) {}

        void set(const char* key, const PSObject& value) 
        {
            dict->put(key, value);
        }

        bool get(const char* key, PSObject& out) const {
            return dict->get(key, out);
        }

        static PSFontHandle createFromSize(const PSFontFaceHandle& face, double size) {
            if (!face) return nullptr;

            auto font = std::make_shared<PSFont>();
            font->face = face;
            font->pointSize = size;

            // scale face matrix by size
            font->transform = face->fDefaultMatrix;
            font->transform.scale(size,size);

            font->set("FontName", PSObject::fromString(face->fName));
            font->set("FontMatrix", PSObject::fromMatrix(font->transform));
            font->set("PointSize", PSObject::fromReal(size));

            return font;
        }

        static PSFontHandle createFromMatrix(const PSFontFaceHandle& face, const PSMatrix& m) {
            if (!face) return nullptr;

            auto font = std::make_shared<PSFont>();
            font->face = face;
            font->transform = m;

            font->set("FontName", PSObject::fromString(face->fName));
            font->set("FontMatrix", PSObject::fromMatrix(m));

            return font;
        }
    };

} // namespace waavs
