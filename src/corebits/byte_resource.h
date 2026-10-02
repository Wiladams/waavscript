// byte_resource.h
#pragma once

#include "lang_span.h"
#include "core_nametable.h"

#include <memory>
#include <utility>

namespace waavs
{


    class ByteResource
    {
        ByteSpan fData;
        std::shared_ptr<const void> fOwner;
        InternedKey fSourceLocation{ nullptr };

    public:
        ByteResource() = default;

        template<typename T>
        ByteResource(ByteSpan data, std::shared_ptr<T> owner, InternedKey sourceLocation = nullptr) noexcept
            : fData(data)
            , fOwner(std::move(owner))
            , fSourceLocation(sourceLocation)
        {}

        [[nodiscard]] bool isValid() const noexcept { return bool(fData); }
        explicit operator bool() const noexcept { return isValid(); }

        [[nodiscard]] ByteSpan data() const noexcept { return fData; }
        [[nodiscard]] size_t size() const noexcept { return fData.size(); }
        [[nodiscard]] InternedKey sourceLocation() const noexcept { return fSourceLocation; }
    };

}