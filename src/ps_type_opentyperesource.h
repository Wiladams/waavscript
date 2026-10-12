#pragma once

#include <memory>
#include <vector>

#include "pscore.h"
#include "ps_type_fontresource.h"
#include "ps_type_dictionary.h"

namespace waavs
{

        /*
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
    */


    /*
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
*/
}