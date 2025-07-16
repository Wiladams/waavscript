#pragma once

#include <map>
#include <memory>
#include <string>

#include "ocspan.h"

// Names are a key aspect of the PostScript runtime.  Names are used as keys 
// in dictionaries, and dictionaries are the primary data structure that 
// holds everything else together.
// Comparisons need to be quick, so we use interned string values.
// This allows comparisons to be nothing more than a comparing two integer 
// values essentially.
//
// PSNameTable - handles the mapping of a string to a stable pointer
// PSName - a wrapper around the interned string pointer, immutable and non-inheritable

// global name table for interned strings.  Anything that is to be a name used
// in a table as a key, should be interned here.
namespace waavs {

    struct PSNameTable {
    private:
        std::map<std::string, const char*> pool;

        const char* intern(std::string_view sv) {
            // Attempt to insert a new entry; try_emplace does nothing if key already exists
            auto [it, inserted] = pool.try_emplace(std::string(sv), nullptr);

            if (inserted) {
                // Assign the stable c_str() from the key (std::string stored in the map)
                it->second = it->first.c_str();
            }

            return it->second;
        }

        const char* intern(const OctetCursor& span) { return intern(std::string_view(reinterpret_cast<const char*>(span.data()), span.size())); }
        const char* intern(const char* cstr) { return intern(std::string_view(cstr)); }

        static PSNameTable* getTable() {
            static std::unique_ptr<PSNameTable> gTable = std::make_unique<PSNameTable>();
            return gTable.get();
        }

    public:
        // NOTE::
        // These should only be used by things inside pscore.h
        // there might ba couple of exceptions, like the cvn operator
        // but for the most part, sting interning should be an internal thing
        static const char* INTERN(const OctetCursor& span) { return getTable()->intern(span); }
        static const char* INTERN(const char* cstr) { return getTable()->intern(cstr ? cstr : ""); }
        static const char* INTERN(const char* ptr, size_t len) { return getTable()->intern(OctetCursor(ptr, len)); }
    };

    //----------------------
    // PSName
    // 
    // Use for names.  Represented by an interned string
    // This class is not intended to be inherited from, and is not mutable
    // a PSName can be passed around as a value type, as it does not own the string data
    // and it is small enough to be trivially copied.
    //-----------------------
    struct PSName {
    private:
        const char* fData; // Interned name, always stable

    public:
        PSName() :fData(nullptr) {
           // printf("PSName default constructor called\n");
        }

        // construct from a null terminated string
        PSName(const char* name) noexcept
            : fData(PSNameTable::INTERN(name)) {
        }

        PSName(const char *ptr, size_t len) noexcept
            : fData(PSNameTable::INTERN(ptr, len)) {
        }

        PSName(const OctetCursor& span) noexcept
            : fData(PSNameTable::INTERN(span)) {
        }

        // comparison betwen two names is as easy as comparing
        // pointers, as they will have the same interning pointer
        // if they are equal.
        bool operator==(const PSName& other) const noexcept {
            return fData == other.fData; // Compare pointers
        }

        // null terminated string for convenience
        const char* c_str() const noexcept {return fData;}

        // a null value is invalid
        bool isValid() const noexcept 
        {
            return fData != nullptr;
        }
    };
}