#pragma once

#include "pscore.h"
#include "psname.h"

namespace waavs {

    // A collection made explicitly to support PSObjects that are
    // accessed by PSName keys.
    struct PSDictEntry {
        PSName key;
        PSObject value;

        bool isEmpty() const noexcept {
            return !key.isValid();
        }
    };

    class PSDictTable {
        PSDictTable() = delete;

    public:
        PSDictTable(size_t initialCapacity)
            : fCapacity(initialCapacity), fCount(0)
        {
            fEntries = new PSDictEntry[fCapacity](); // zero-initialized
        }

        ~PSDictTable() {
            delete[] fEntries;
        }

        bool put(PSName key, const PSObject& value) {
            if (loadFactor() > 0.75)
                grow();

            size_t index = findSlot(key);
            if (fEntries[index].isEmpty()) {
                fCount++;
            }
            fEntries[index].key = key;
            fEntries[index].value = value;
            return true;
        }

        bool get(PSName key, PSObject& outValue) const {
            size_t index = findSlot(key);
            if (fEntries[index].key == key) {
                outValue = fEntries[index].value;
                return true;
            }
            return false;
        }

        size_t size() const { return fCount; }

        void clear() {
            for (size_t i = 0; i < fCapacity; ++i) {
                fEntries[i].key = PSName();       // invalidate key
                fEntries[i].value.reset();        // drop shared_ptrs, destroy objects
            }
            fCount = 0;
        }

        template <typename Fn>
        void forEach(Fn&& fn) {
            for (size_t i = 0; i < fCapacity; ++i) {
                if (!fEntries[i].isEmpty()) {
                    if (!fn(fEntries[i].key, fEntries[i].value)) break;
                }
            }
        }

    private:
        PSDictEntry* fEntries = nullptr;
        size_t fCapacity = 0;
        size_t fCount = 0;

        float loadFactor() const {
            return static_cast<float>(fCount) / static_cast<float>(fCapacity);
        }

        void grow() {
            PSDictEntry* old = fEntries;
            size_t oldCap = fCapacity;

            fCapacity *= 2;
            fEntries = new PSDictEntry[fCapacity]();

            fCount = 0;
            for (size_t i = 0; i < oldCap; ++i) {
                if (!old[i].isEmpty()) {
                    put(old[i].key, old[i].value);
                }
            }
            delete[] old;
        }

        // try to find the slot for the key using linear probing
        size_t findSlot(PSName key) const {
            size_t hash = reinterpret_cast<size_t>(key.c_str());
            size_t index = hash % fCapacity;

            while (!fEntries[index].isEmpty()) {
                if (fEntries[index].key == key)
                    return index;

                index = (index + 1) % fCapacity;
            }
            return index;
        }



    };

    // --------------------
    // PSDictionary
    // --------------------
    struct PSDictionary {
    private:
        // Actual storage for the dictionary entries
        // the key is PSName, which is an interned string
        PSDictTable fEntries;

        PSDictionary() = default;

        PSDictionary(size_t initialSize=32) : fEntries(initialSize){}


    public:

        static PSDictionaryHandle create(size_t initialSize = 32) {
            auto ptr = std::shared_ptr<PSDictionary>(new PSDictionary(initialSize));

            return ptr;
        }

        size_t size() const { return fEntries.size(); }

        bool put(const PSName& key, const PSObject& value) {
            return fEntries.put(key, value);
        }
        bool get(const PSName& key, PSObject& out) const {
            return fEntries.get(key, out);
        }

        bool copyEntryFrom(const PSDictionary& other, const PSName& key) {
            PSObject value;
            if (!other.get(key, value)) return false;

            fEntries.put(key, value);
            return true;
        }

        bool contains(const PSName& key) const {
            PSObject dummy;
            return fEntries.get(key, dummy);
        }

        void clear() {
            fEntries.clear();
        }

        // Apply a function to each entry in the dictionary.
        bool forEach(std::function<bool(PSName, PSObject&)> fn) {
            fEntries.forEach(fn);
            return true;
        }
    };
} // namespace waavs

