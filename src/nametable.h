#pragma once

#include <map>
#include <string>
#include <string_view>
#include <memory>

#include "ocspan.h"



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

        const char* intern(const OctetCursor& span) {  return intern(std::string_view(reinterpret_cast<const char*>(span.data()), span.size()));  }
        const char* intern(const char* cstr) {  return intern(std::string_view(cstr));  }

        static PSNameTable* getTable() {
            static std::unique_ptr<PSNameTable> gTable = std::make_unique<PSNameTable>();
            return gTable.get();
        }

    public:
        // NOTE::
        // These should only be used by things inside pscore.h
        // there might ba couple of exceptions, like the cvn operator
        // but for the most part, sting interning should be an internal thing
        static const char* INTERN(const OctetCursor& span) { return getTable()->intern(span);}
        static const char* INTERN(const char* cstr) { return getTable()->intern(cstr);}
    };


}
