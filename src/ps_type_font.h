#pragma once

#include <memory>

#include "pscore.h"
#include "ps_type_fontresource.h"
#include "ps_type_matrix.h"
#include "ps_type_dictionary.h"

namespace waavs
{
    struct PSFont : public PSDictionary
    {
        std::shared_ptr<PSFontResource> resource;

        PSFont() = default;

        PSFont(std::shared_ptr<PSFontResource> fontResource)
            : PSDictionary(32),
            resource(std::move(fontResource))
        {}

        // Construct a font from an existing dictionary.
        // Preserve the dictionary's FontMatrix.
        PSFont(std::shared_ptr<PSFontResource> fontResource, std::shared_ptr<PSDictionary> dict)
            : PSDictionary(32),
            resource(std::move(fontResource))
        {
            if (dict)
                copyEntriesFrom(*dict);
        }

        // Construct a derived font with a replacement effective FontMatrix.
        PSFont(std::shared_ptr<PSFontResource> fontResource, std::shared_ptr<PSDictionary> dict, const PSMatrix& matrix)
            : PSDictionary(32),
            resource(std::move(fontResource))
        {
            if (dict)
                copyEntriesFrom(*dict);

            put("FontMatrix", PSObject::fromMatrix(matrix));
        }

        PSMatrix matrix() const
        {
            PSObject obj;

            if (get(PSName("FontMatrix"), obj) && obj.isMatrix())
                return obj.asMatrix();

            return PSMatrix();
        }

        bool valid() const noexcept
        {
            return static_cast<bool>(resource);
        }

        std::shared_ptr<PSFontResource> getResource() const noexcept
        {
            return resource;
        }

        static std::shared_ptr<PSFont> create(std::shared_ptr<PSFontResource> resource)
        {
            return std::make_shared<PSFont>(std::move(resource));
        }

        static std::shared_ptr<PSFont> create(std::shared_ptr<PSFontResource> resource, PSDictionaryHandle dict)
        {
            return std::make_shared<PSFont>(std::move(resource), std::move(dict));
        }

        static std::shared_ptr<PSFont> create(std::shared_ptr<PSFontResource> resource, PSDictionaryHandle dict, const PSMatrix& matrix)
        {
            return std::make_shared<PSFont>(std::move(resource), std::move(dict), matrix);
        }
    };

    inline PSFontHandle getNativeFont(const PSObject& obj)
    {
        if (!obj.isDictionary())
            return nullptr;

        return std::dynamic_pointer_cast<PSFont>(obj.asDictionary());
    }
}