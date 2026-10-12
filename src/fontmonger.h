#pragma once

#include <vector>
#include <algorithm>

#include <blend2d/blend2d.h>
#include <blend2d/core/fontmanager.h>

#include "strutil.h"

#include "pscore.h"
#include "psvm.h"
#include "ps_type_font.h"
#include "ps_type_opentypefontresource.h"

namespace waavs {

    // Create a PostScript name from family and subfamily names.
    // The PostScript name is constructed as follows:
    // 1. Strip spaces from family name and convert to lowercase.
    // 2. If subfamily is empty or "regular", do not append it.
    // 3. If subfamily is non-empty and not "regular", append it after a dash, also in lowercase.
    //
    /*
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
    */
} // namespace waavs



namespace waavs {

    /*
    class FontMonger {
    public:
        using Predicate = std::function<bool(const PSFontFaceHandle)>;

    private:



    public:
        FontMonger() {
            //fontFacesByName = PSDictionary::create();
        }



        static FontMonger* instance() {
            static std::unique_ptr<FontMonger> gInstance;
            if (gInstance == nullptr) {
                gInstance = std::unique_ptr<FontMonger>(new FontMonger);
            } 
            
            return gInstance.get();
        }
    };
    */

    // Scan all the fonts in the specified directory, making them
    // known to the FontMonger.
    //

    inline size_t loadFontsInDirectory(PSVirtualMachine* vm, const char* dirpath)
    {
        namespace fs = std::filesystem;
        size_t count = 0;
        for (const auto& entry : fs::directory_iterator(dirpath)) {
            if (!entry.is_regular_file()) continue;

            auto ext = entry.path().extension().string();
            if (ext == ".ttf" || ext == ".otf" || ext == ".ttc") {
                PSObject pathObj = PSObject::fromString(entry.path().string().c_str());
                vm->opStack().push(pathObj);
                if (loadFontResource(*vm))
                    ++count;
            }
        }
        return count;
    }
} // namespace waavs
