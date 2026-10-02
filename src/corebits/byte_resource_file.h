#pragma once

#include "byte_resource.h"
#include "read_only_mapped_file.h"
#include "core_nametable.h"

#include <filesystem>

namespace waavs
{
    namespace fs = std::filesystem;


    // ================================================================
    // Create an interned, absolute source-location string.
    // ================================================================

    inline InternedKey createNormalizedPath(const fs::path& path)
    {
        std::error_code ec;

        fs::path absolutePath = fs::absolute(path, ec);

        const fs::path& result = ec ? path : absolutePath;

        const std::string text = result.string();

        return WSNameSet::INTERN(text.c_str());
    }

    inline InternedKey createNormalizedPath(const char* path)
    {
        if (!path || !*path)
            return nullptr;

        return createNormalizedPath(fs::path(path));
    }


    inline bool readByteResource(const fs::path& path, ByteResource& out)
    {
        out = {};

        auto mapped = ReadOnlyMappedFile::open(path);

        if (!mapped || mapped->empty())
            return false;

        auto owner = std::make_shared<ReadOnlyMappedFile>(std::move(*mapped));

        out = ByteResource(owner->bytes(), owner, createNormalizedPath(path));

        return true;
    }


    using FontResource = ByteResource;
    using DatabaseResource = ByteResource;

}

