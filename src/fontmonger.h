#pragma once

#include <vector>
#include <algorithm>

#include <blend2d/blend2d.h>
#include <blend2d/blend2d/fontmanager.h>

#include "strutil.h"

#include "pscore.h"
#include "psvm.h"
#include "ps_type_font.h"


namespace waavs {

    // Create a PostScript name from family and subfamily names.
    // The PostScript name is constructed as follows:
    // 1. Strip spaces from family name and convert to lowercase.
    // 2. If subfamily is empty or "regular", do not append it.
    // 3. If subfamily is non-empty and not "regular", append it after a dash, also in lowercase.
    //
    static PSName createPostScriptName(const PSName& family, const PSName& subfamily) {
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
        return PSName(reinterpret_cast<const char*>(temp.data(), temp.length()));
    }

} // namespace waavs



namespace waavs {


    class FontMonger {
    public:
        using Predicate = std::function<bool(const PSFontFaceHandle)>;

    private:

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
            //fontFacesByName = PSDictionary::create();
        }


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
        // Scan a font file and extract metadata.
        // Returns true if the font was successfully scanned and metadata extracted.
        bool loadFontResource(PSVirtualMachine &vm) {
            BLFontFace face;

            auto& s = vm.opStack();
            auto& estk = vm.execStack();

            // pop the path from the stack
            if (s.size() < 1) {
                vm.error("loadFontResource: stackunderflow");
                return false;
            }

            PSObject pathObj;
            s.pop(pathObj);
            
            if (!pathObj.isString()) {
                vm.error("loadFontResource: typecheck; Expected a string on the stack for font path");
                return false;
            }

            auto filePathStr = pathObj.asString().toString();
            const char *filePath = filePathStr.c_str();
            if (face.createFromFile(filePath) != BL_SUCCESS || !face.isValid())
                return vm.error("createFromFile error");


            const BLFontDesignMetrics &dm = face.designMetrics();

            // bounding box is an array of 4 reals: [llx, lly, urx, ury]
            auto arr = PSArray::create();
            arr->append(PSObject::fromReal(dm.glyphBoundingBox.x0)); // llx
            arr->append(PSObject::fromReal(dm.glyphBoundingBox.y0)); // lly
            arr->append(PSObject::fromReal(dm.glyphBoundingBox.x1)); // urx
            arr->append(PSObject::fromReal(dm.glyphBoundingBox.y1)); // ury

            // FontMatrix can be built from unitsPerEm, which is what's in the
            // design metrics
            PSMatrix fontMatrix(1.0/dm.unitsPerEm, 0.0, 0.0, 1.0/dm.unitsPerEm, 0.0, 0.0);

            const char* psName = toLowerIntern(face.postScriptName());
            const char* famName = toLowerIntern(face.familyName());
            const char* subfamName = toLowerIntern(face.subfamilyName());

            auto psface = PSFontFace::create();
            psface->set("FontFile", pathObj);
            psface->set("FontName", PSObject::fromName(PSName(psName)));
            psface->set("FamilyName", PSObject::fromName(PSName(famName)));
            psface->set("SubfamilyName", PSObject::fromName(PSName(subfamName)));

            psface->set("Weight", PSObject::fromInt(face.weight()));
            psface->set("Stretch", PSObject::fromInt(face.stretch()));
            psface->set("Style", PSObject::fromInt(face.style()));

            psface->set("FontBBox", PSObject::fromArray(arr));
            psface->set("UnitsPerEm", PSObject::fromInt(face.unitsPerEm()));
            psface->set("FontMatrix", PSObject::fromMatrix(fontMatrix));
            //psface->set("FontType", PSObject::fromInt(face.fontType()));

            // We want to save the font into the ResourceDirectory.  We'll use the 
            // already available 'defineresource' operator to do this.
            // Key: PostScript name (interned, lowercase)
            // Value: font face handle 
            // Category: Font Category
            // key category value  defineresource
            s.pushLiteralName(psName);
            s.pushLiteralName("Font");
            s.push(PSObject::fromFontFace(psface));

            estk.pushExecName("defineresource");

            vm.run();


            // value left on stack, so pop that, we don't need it
            PSObject resObj;
            if (!s.pop(resObj))
                return vm.error("loadFontResource: stackunderflow");

            return true;
        }

        static bool createFont(const PSObject &faceObj, float sz, PSObject &fontObj)
        {
            // get the FontFile out of the face dictionary
            // create a BLFontFace from it
            auto face = faceObj.asFontFace();
            if (!face) {
                return false; // Error: Not a valid font face object
            }

            //writeObjectDeep(faceObj);

            PSObject fontFileObj;
            if (!face->get("FontFile", fontFileObj) || !fontFileObj.isString()) {
                return false; // Error: FontFile not found or not a string
            }

            PSString fontFileStr = fontFileObj.asString();
            auto fontFileName = fontFileStr.toString();
            const char* fontFilePath = fontFileName.c_str();
            //const char* fontFilePath = fontFileObj.asString().toString().c_str();

            BLFontFace fFace;
            BLResult result = fFace.createFromFile(fontFilePath);

            if (result != BL_SUCCESS || !fFace.isValid()) {
                //delete fFace; // Clean up if creation failed
                return false; // Error: Failed to create BLFontFace
            }


            BLFont* font = new BLFont();

            result = font->createFromFace(fFace, sz); // Create a font from the face at the specified size

            if (result != BL_SUCCESS || !font->isValid()) {
                delete font; // Clean up if creation failed
                return false; // Error: Failed to create BLFont
            }
            
            PSFontHandle fontHandle = PSFont::create(font);

            fontObj.resetFromFont(fontHandle);

            return true;

        }


        /*
        // Find metadata by a predicate function.
        // This allows you to use any algorithm to select a font based on the 
        // available metadata.
        // 
        // Return: 
        //      false if nothing found, or at end of entries
        //      true if found, and meta data is filled in
        bool findFontMetaNext(Predicate pred, PSFontFaceHandle& metaOut, uint32_t& cookie) const {
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

        bool findFontMetaFirst(Predicate pred, PSFontFaceHandle& metaOut) const {
            uint32_t cookie = 0;
            return findFontMetaNext(pred, metaOut, cookie);
        }

        bool FontMonger::findFontMetaByName(const PSName & name, PSFontFaceHandle& metaOut) const {
            // Normalize the name by interning the lowercased version
            //const char* interned = PSNameTable::INTERN(name);  // should already be lowercase if preprocessed
            uint32_t cookie = 0;

            auto matchByPostscriptName = [=](const PSFontFaceHandle& m) {
                // If it doesn't have a FontName, skip it
                PSObject nameObj;
                if (!m->get("FontName", nameObj))
                    return false;

                return nameObj.isName() &&
                    nameObj.asName() == name;
                };

            return findFontMetaNext(matchByPostscriptName, metaOut, cookie);
        }
        */

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
        /*
        bool findFontFaceByName(const PSName &name, PSObject& out) 
        {            
            // Check if already cached, then just return that
            if (fontFacesByName->get(name, out)) {
                return true;
            }

            // If not already cached, we need to load the BLFontFace again
            // and hold onto it in the PSFontFaceHandle
            PSFontFaceHandle meta;
            if (!findFontMetaByName(name, meta))
                return false;


            // Attempt to load font face
            BLFontFace* fFace = new BLFontFace();


            PSObject fontPath;
            meta->get("FontFile", fontPath);
            const char* path = fontPath.asString().toString().c_str();
            BLResult result = fFace->createFromFile(path);
            if (result != BL_SUCCESS || !fFace->isValid())
                return false;

            meta->setSystemHandle(fFace);

            // Cache it
            out.resetFromFontFace(meta);
            PSObject psNameObj;
            meta->get("FontName", psNameObj);
            fontFacesByName->put(psNameObj.asName(), out);

            return true;
        }

        // Use a predicate, find a font face
        bool findFont(Predicate pred, PSObject& outObj) {
            uint32_t cookie = 0;
            PSFontFaceHandle meta;
            if (findFontMetaNext(pred, meta, cookie))
                return true;

            return false;
        }
        */

        static FontMonger* instance() {
            static std::unique_ptr<FontMonger> gInstance;
            if (gInstance == nullptr) {
                gInstance = std::unique_ptr<FontMonger>(new FontMonger);
            } 
            
            return gInstance.get();
        }
    };

} // namespace waavs
