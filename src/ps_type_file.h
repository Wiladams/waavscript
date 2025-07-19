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

    protected:
        PSFile() = default;
        OctetCursor fCursor;

    public:
        virtual bool hasCursor() const { return false; }
        virtual OctetCursor& getCursor()   {return fCursor;}

        virtual size_t size() const  { return 0; }
        virtual bool isValid() const  {  return false;  }



        // Read one byte
        virtual bool readByte(uint8_t& out)  { return false; }
        virtual bool readBytes(uint8_t* out, size_t count)  { return false; }
        virtual bool flush() { return true; }

        // Positioning
        virtual size_t position() const  { return 0; }
        virtual bool setPosition(size_t pos) {return false;}
        virtual void rewind()  {}

        virtual bool isEOF() const { return true; } 

        virtual void finalize() {}
    };


    //====================================================
    // Memory File
    //=====================================================
    class PSMemoryFile : public PSFile
    {
    protected:
        OctetCursor fOrigin; // Original cursor for rewinding

        PSMemoryFile() = default;

    public:
        explicit PSMemoryFile(const OctetCursor& data)
            : fOrigin(data)
        {
            fCursor = fOrigin;
        }

        bool hasCursor() const override { return true; }


        size_t size() const override { return fOrigin.size(); }

        bool isValid() const override { return fOrigin.size() > 0; }

        // Read one byte
        bool readByte(uint8_t& out) override {
            if (fCursor.empty())
                return false;
            out = *fCursor;
            ++fCursor;

            return true;
        }

        bool readBytes(uint8_t* out, size_t count) override {
            if (fCursor.size() < count)
                return false;

            std::memcpy(out, fCursor.begin(), count);
            fCursor.advance(count);

            return true;
        }

        // Positioning
        size_t position() const override
        {
            return fCursor.begin() - fOrigin.begin();
        }
        
        bool setPosition(size_t pos) override 
        {
            if (pos > fOrigin.size())
                return false;   // range check, out of bounds

            fCursor = OctetCursor(fOrigin.data() + pos, fOrigin.size() - pos);

            return true;
        }
        void rewind() override
        {
            fCursor = fOrigin;
        }


        bool isEOF() const override { return fCursor.empty(); }

        static std::shared_ptr<PSMemoryFile> create(const OctetCursor &dataSrc)
        {
            return std::shared_ptr<PSMemoryFile>(new PSMemoryFile(dataSrc));
        }
    };

    //====================================================
    //
    //====================================================

    class PSDiskFile : public PSMemoryFile
    {
    private:
        std::shared_ptr<MappedFile> fMapped; // Mapped file handle

        // Private constructor; use create()
        PSDiskFile(std::shared_ptr<MappedFile> mf)
            : PSMemoryFile(OctetCursor(mf->data(), mf->size())),
            fMapped(std::move(mf))
        {
            if (fMapped && fMapped->isValid())
            {
                fOrigin = OctetCursor(fMapped->data(), fMapped->size());
                fCursor = fOrigin;
            }
        }

    public:

        //==================================================
        // Factory constructor
        //===================================================
        static std::shared_ptr<PSFile> create(std::shared_ptr<MappedFile> mf)
        {
            if (!mf || !mf->isValid())
                return {};
            return std::shared_ptr<PSFile>(new PSDiskFile(std::move(mf)));
        }

        static std::shared_ptr<PSFile> create(const std::string& fname, const std::string& amode)
        {
            uint32_t desiredAccess = GENERIC_READ;
            uint32_t shareMode = FILE_SHARE_READ;
            uint32_t disposition = OPEN_EXISTING;

            // Only allow read mode for now
            if (amode != "r")
                return {};

            std::shared_ptr<MappedFile> mf = MappedFile::create_shared(fname, desiredAccess, shareMode, disposition);

            return create(std::move(mf));
        }


        static std::shared_ptr<PSFile> create(const PSString& filename, const PSString& access)
        {
            std::string fname(reinterpret_cast<const char*>(filename.data()), filename.length());
            std::string amode(reinterpret_cast<const char*>(access.data()), access.length());

            return create(fname, amode);
        }


    };



} // namespace waavs
