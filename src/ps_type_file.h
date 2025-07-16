#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "mappedfile.h"
#include "ocspan.h"
#include "ps_type_string.h"


namespace waavs {

    class PSFile {
    private:
        std::shared_ptr<MappedFile> mapped;
        OctetCursor cursor;
        std::string mode;

        // Private constructor; use create()
        PSFile(std::shared_ptr<MappedFile> mf, std::string access)
            : mapped(std::move(mf)), mode(std::move(access)) {
            if (mapped && mapped->isValid())
                cursor = OctetCursor(mapped->data(), mapped->size());
        }

    public:
        // Factory constructor
        static std::shared_ptr<PSFile> create(const PSString& filename, const PSString& access) {
            std::string fname(reinterpret_cast<const char*>(filename.data()), filename.length());
            std::string amode(reinterpret_cast<const char*>(access.data()), access.length());

            uint32_t desiredAccess = GENERIC_READ;
            uint32_t shareMode = FILE_SHARE_READ;
            uint32_t disposition = OPEN_EXISTING;

            // Only allow read mode for now
            if (amode != "r")
                return {};

            std::shared_ptr<MappedFile> mf = MappedFile::create_shared(fname, desiredAccess, shareMode, disposition);
            if (!mf || !mf->isValid())
                return {};

            return std::shared_ptr<PSFile>(new PSFile(std::move(mf), amode));
        }

        // Read one byte
        bool readByte(uint8_t& out) noexcept {
            if (cursor.empty())
                return false;
            out = *cursor;
            ++cursor;
            return true;
        }

        // Positioning
        size_t position() const noexcept {
            return static_cast<size_t>(cursor.begin() - reinterpret_cast<const uint8_t*>(mapped->data()));
        }

        bool setPosition(size_t pos) noexcept {
            if (!mapped || pos > mapped->size()) return false;
            cursor = OctetCursor(((uint8_t *)mapped->data()) + pos, mapped->size() - pos);
            return true;
        }

        void rewind() noexcept {
            if (mapped)
                cursor = OctetCursor(mapped->data(), mapped->size());
        }

        size_t size() const noexcept {
            return mapped ? mapped->size() : 0;
        }

        bool isValid() const noexcept {
            return mapped && mapped->isValid();
        }

        const OctetCursor& getCursor() const noexcept { return cursor; }
        OctetCursor& getCursor() noexcept { return cursor; }
    };

} // namespace waavs
