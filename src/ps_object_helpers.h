// ps_object_helpers.h

#pragma once

#include "pscore.h"
#include "ps_type_font.h"

namespace waavs 
{
    /*
    inline bool objectAsDictionary(const PSObject& obj, PSDictionaryHandle& dict)
    {
        if (obj.isDictionary()) {
            dict = obj.asDictionary();
            return true;
        }

        if (obj.isFont()) {
            PSFontHandle font = obj.asFont();
            if (!font)
                return false;

            dict = font->getDictionary();
            return static_cast<bool>(dict);
        }

        return false;
    }
    */
} // namespace waavs