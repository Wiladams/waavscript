#pragma once

#include <vector>
#include <algorithm>

#include <blend2d/blend2d.h>
#include <blend2d/blend2d/fontmanager.h>

#include "pscore.h"
#include "psfont.h"


namespace waavs {


        // Portable replacement for POSIX strncasecmp
    inline int pstrncasecmp(const char* s1, const char* s2, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            unsigned char c1 = static_cast<unsigned char>(s1[i]);
            unsigned char c2 = static_cast<unsigned char>(s2[i]);

            // Null terminator ends comparison
            if (c1 == '\0' || c2 == '\0') {
                return c1 - c2;
            }

            // Compare lowercased characters
            int diff = std::tolower(c1) - std::tolower(c2);
            if (diff != 0)
                return diff;
        }
        return 0;
    }


    // Create a PostScript name from family and subfamily names.
    // The PostScript name is constructed as follows:
    // 1. Strip spaces from family name and convert to lowercase.
    // 2. If subfamily is empty or "regular", do not append it.
    // 3. If subfamily is non-empty and not "regular", append it after a dash, also in lowercase.
    //
    PSName postScriptName(const PSName& family, const PSName& subfamily) {
        const char* fam = family.c_str();
        const char* sub = subfamily.c_str();

        size_t famLen = std::strlen(fam);
        size_t subLen = std::strlen(sub);

        // Allocate enough space: famLen + 1 (dash) + subLen
        PSString temp(famLen + 1 + subLen);  // PSString defaults to zeroed memory
        size_t index = 0;

        // Copy family name: strip spaces, convert to lowercase
        for (size_t i = 0; i < famLen; ++i) {
            char c = fam[i];
            if (c != ' ') {
                temp.put(static_cast<uint32_t>(index++), static_cast<uint8_t>(std::tolower(c)));
            }
        }

        // Normalize subfamily: treat empty or "regular" as no suffix
        bool hasSubfamily = false;
        for (size_t i = 0; i < subLen; ++i) {
            if (sub[i] != ' ' && std::tolower(sub[i]) != 'r') {
                hasSubfamily = true;
                break;
            }
        }
        if (subLen > 0 && !(subLen == 7 && pstrncasecmp(sub, "regular", 7) == 0)) {
            hasSubfamily = true;
        }

        // Append dash and subfamily if needed
        if (hasSubfamily) {
            temp.put(static_cast<uint32_t>(index++), '-');
            for (size_t i = 0; i < subLen; ++i) {
                char c = sub[i];
                if (c != ' ') {
                    temp.put(static_cast<uint32_t>(index++), static_cast<uint8_t>(std::tolower(c)));
                }
            }
        }

        temp.setLength(static_cast<uint32_t>(index));
        return PSName(PSNameTable::INTERN(reinterpret_cast<const char*>(temp.data()), temp.length()));
    }

} // namespace waavs



namespace waavs {

    struct FontMetadata {
        PSName postscriptName; // interned, lowercase
        PSName familyName;     // interned, lowercase
        PSName subfamily;      // interned, lowercase
        PSName path;           // interned (optional: lowercase)
        uint32_t weight = BL_FONT_WEIGHT_NORMAL;
        uint32_t stretch = BL_FONT_STRETCH_NORMAL;
        uint32_t style = BL_FONT_STYLE_NORMAL;
        std::shared_ptr<BLFontFace> fFace;
    };

    class FontMonger {
    public:
        using Predicate = std::function<bool(const FontMetadata&)>;

    private:
        std::vector<FontMetadata> fMeta;
        PSDictionaryHandle fPostScriptIndex;    // maps postscript name to index in fMeta
        PSDictionaryHandle fontFacesByName;     // maps postscript name to PSFontFaceHandle 

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
        FontMonger() {
            fPostScriptIndex = PSDictionary::create();
            fontFacesByName = PSDictionary::create();
        }

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
            const char* subfamName = toLowerIntern(face.subfamilyName());
            const char* pathName = PSNameTable::INTERN(path); // no need to lowercase path

            FontMetadata meta{};
            meta.postscriptName = psName;
            meta.familyName = famName;
            meta.subfamily = subfamName;
            meta.path = pathName;

            meta.weight = face.weight();
            meta.stretch = face.stretch();
            meta.style = face.style();

            printf("Family: %-16s, SubFamily: %-16s PostScript: %20s, Path: %s\n", 
                meta.familyName, meta.subfamily, meta.postscriptName, meta.path);

            size_t index = fMeta.size();
            fMeta.push_back(meta);
            fPostScriptIndex->put(psName, PSObject::fromInt(index));

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

        bool FontMonger::findFontMetaByName(const PSName & name, FontMetadata& metaOut) const {
            // Normalize the name by interning the lowercased version
            //const char* interned = PSNameTable::INTERN(name);  // should already be lowercase if preprocessed
            uint32_t cookie = 0;

            auto matchByPostscriptName = [=](const FontMetadata& m) {
                return m.postscriptName == name;
                };

            return findFontMetaNext(matchByPostscriptName, metaOut, cookie);
        }


        /*
        //======================================================================
        // Finding fonts by metadata
        //=======================================================================
        // 
        bool findFontFaceByPredicate(Predicate pred, PSFontFaceHandle& out)
        {
            uint32_t cookie = 0;
            FontMetadata meta;
            if (!findFontMetaNext(pred, meta, cookie))
                return false;

            // If the meta already contains BLFontFace, then return that directly
            if (!meta.fFace) {
                meta.fFace = std::make_shared<BLFontFace>();
                BLResult result = meta.fFace.get()->createFromFile(meta.path);
                if (result != BL_SUCCESS || !meta.fFace->isValid())
                    return false;
            }


            // Construct and register PSFontFace
            // Create handle that will ultimately be returned
            auto handle = std::shared_ptr<PSFontFace>(new PSFontFace(meta));
           // handle->fName = meta.postscriptName;
            //handle->fDefaultMatrix.reset();
            //handle->set("FontName", PSObject::fromString(meta.postscriptName));
            //handle->set("FontMatrix", PSObject::fromMatrix(handle->fDefaultMatrix));
            //handle->set("FontFile", PSObject::fromString(meta.path));

            // Cache it
            fontFacesByName->put(meta.postscriptName, PSObject::fromFontFace(handle));
            out = handle;
            return true;
        }
        */

        bool findFontFaceByName(const PSName &name, PSObject& out) 
        {
            //const char* internedName = PSNameTable::INTERN(name);
            
            // Check if already cached, then just return that
            if (fontFacesByName->get(name, out)) {
                return true;
            }


            FontMetadata meta;
            if (!findFontMetaByName(name, meta))
                return false;


            // Attempt to load font face
            meta.fFace = std::make_shared<BLFontFace>();
            BLResult result = meta.fFace.get()->createFromFile(meta.path.c_str());
            if (result != BL_SUCCESS || !meta.fFace->isValid())
                return false;

            // Construct and register PSFontFace
            // Create handle that will ultimately be returned
            auto handle = std::make_shared<PSFontFace>();
            //handle->fName = meta.postscriptName;
            //handle->fDefaultMatrix.reset();
            handle->set("FontName", PSObject::fromName(meta.postscriptName));
            //handle->fDict->copyEntryFrom("FontMatrix", );
            handle->set("FontFile", PSObject::fromName(meta.path));

            // Cache it
            out.resetFromFontFace(handle);
            fontFacesByName->put(meta.postscriptName,out);

            return true;
        }

        // Use a predicate, find a font face
        bool findFont(Predicate pred, PSObject& outObj) {
            uint32_t cookie = 0;
            FontMetadata meta;
            if (!findFontMetaNext(pred, meta, cookie))
                return false;

            return findFontFaceByName(meta.postscriptName, outObj);
        }

        static FontMonger& instance() {
            static FontMonger instance;
            return instance;
        }
    };

} // namespace waavs
