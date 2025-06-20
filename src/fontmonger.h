#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include <blend2d/blend2d.h>
#include "pscore.h"
#include "psfont.h"

namespace waavs {

    struct FontMetadata {
        const char* postscriptName; // interned, lowercase
        const char* familyName;     // interned, lowercase
        const char* styleName;      // interned, lowercase
        const char* path;           // interned (optional: lowercase)
        uint32_t weight = BL_FONT_WEIGHT_NORMAL;
        uint32_t stretch = BL_FONT_STRETCH_NORMAL;
        uint32_t style = BL_FONT_STYLE_NORMAL;
        BLFontFace* fFace{ nullptr };
    };

    class FontMonger {
    public:
        //using Predicate = bool(*)(const FontMetadata&);
        using Predicate = std::function<bool(const FontMetadata&)>;

    private:
        std::vector<FontMetadata> fMeta;
        std::unordered_map<const char*, size_t> fPostScriptIndex;
        std::unordered_map<const char*, PSFontFaceHandle> fontFacesByName;

        static const char* toLowerIntern(const BLString& str) {
            std::string temp(static_cast<const char*>(str.data()), str.size());
            for (auto& c : temp) c = (char)std::tolower(c);
            return PSNameTable::INTERN(temp.c_str(), temp.size());
        }

        static const char* toLowerIntern(const char* str) {
            std::string temp(str);
            for (auto& c : temp) c = (char)std::tolower(c);
            return PSNameTable::INTERN(temp.c_str(), temp.size());
        }

    public:

        // return how many entries in the metadata table
        size_t count() const { return fMeta.size(); }

        // return the metadata at index i
        const FontMetadata& at(size_t i) const { return fMeta[i]; }

        // Scan a font file and extract metadata.
        // Returns true if the font was successfully scanned and metadata extracted.
        bool scanFontFile(const char* path) {
            BLFontFace face;
            if (face.createFromFile(path) != BL_SUCCESS || !face.isValid())
                return false;

            const char* psName = toLowerIntern(face.postScriptName());
            const char* famName = toLowerIntern(face.familyName());
            const char* pathName = PSNameTable::INTERN(path); // no need to lowercase path

            FontMetadata meta{};
            meta.postscriptName = psName;
            meta.familyName = famName;
            meta.path = pathName;

            meta.weight = face.weight();
            meta.stretch = face.stretch();
            meta.style = face.style();

            printf("Family: %s, PostScript: %s, Path: %s\n", 
                meta.familyName, meta.postscriptName, meta.path);

            size_t index = fMeta.size();
            fMeta.push_back(meta);
            fPostScriptIndex[psName] = index;

            return true;
        }



        // Find metadata by a predicate function.
        // This allows you to use any algorithm to select a font based on the 
        // available metadata.
        // 
        // Return: 
        //      false if nothing found, or at end of entries
        //      true if found, and meta data is filled in
        bool findFontMetaNext(Predicate pred, FontMetadata& metaOut, uint32_t& cookie) const {
            size_t i = static_cast<size_t>(cookie);
            while (i < fMeta.size()) {
                if (pred(fMeta[i])) {
                    metaOut = fMeta[i];
                    cookie = static_cast<uint32_t>(i + 1);
                    return true;
                }
                ++i;
            }
            return false;
        }

        bool findFontMetaFirst(Predicate pred, FontMetadata& metaOut) const {
            uint32_t cookie = 0;
            return findFontMetaNext(pred, metaOut, cookie);
        }

        bool FontMonger::findFontMetaByName(const char* name, FontMetadata& metaOut) const {
            // Normalize the name by interning the lowercased version
            const char* interned = PSNameTable::INTERN(name);  // should already be lowercase if preprocessed
            uint32_t cookie = 0;

            auto matchByPostscriptName = [=](const FontMetadata& m) {
                return m.postscriptName == interned;
                };

            return findFontMetaNext(matchByPostscriptName, metaOut, cookie);
        }



        //======================================================================
        // Finding fonts by metadata
        //=======================================================================
        // 
        bool findFontFaceByName(const char *name, PSFontFaceHandle& out) 
        {
            const char* internedName = PSNameTable::INTERN(name);
            
            // Check if already cached, then just return that
            auto it = fontFacesByName.find(name);
            if (it != fontFacesByName.end()) {
                out = it->second;
                return true;
            }

            FontMetadata meta;
            if (!findFontMetaByName(name, meta))
                return false;

            // Attempt to load font face
            BLFontFace face;
            if (face.createFromFile(meta.path) != BL_SUCCESS || !face.isValid())
                return false;

            // Construct and register PSFontFace
            auto handle = std::make_shared<PSFontFace>();
            handle->fName = meta.postscriptName;
            handle->fDefaultMatrix.reset();
            handle->set("FontName", PSObject::fromString(meta.postscriptName));
            handle->set("FontMatrix", PSObject::fromMatrix(handle->fDefaultMatrix));
            handle->set("FontFile", PSObject::fromString(meta.path));

            // Cache it
            fontFacesByName[meta.postscriptName] = handle;
            out = handle;
            return true;
        }

        // Use a predicate, find a font face
        bool findFont(Predicate pred, PSFontFaceHandle& out) {
            uint32_t cookie = 0;
            FontMetadata meta;
            if (!findFontMetaNext(pred, meta, cookie))
                return false;

            return findFontFaceByName(meta.postscriptName, out);
        }

        static FontMonger& instance() {
            static FontMonger instance;
            return instance;
        }
    };

} // namespace waavs
