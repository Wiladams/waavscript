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

    class PSDictionary {
        PSDictionary() = delete;

    public:
        // Create entries, with a minimum initial capacity
        PSDictionary(size_t initialCapacity)
            : fCapacity(std::max(size_t(4), initialCapacity))
            , fCount(0)
        {
            fEntries = new PSDictEntry[fCapacity](); // zero-initialized
        }

        ~PSDictionary() {
            delete[] fEntries;
        }

        static std::shared_ptr<PSDictionary> create(size_t initialSize = 32) {
            auto ptr = std::shared_ptr<PSDictionary>(new PSDictionary(initialSize));

            return ptr;
        }

        constexpr size_t size() const noexcept { return fCount; }
        constexpr bool empty() const noexcept { return fCount == 0; }   

        // put
        // Place the value into the table 
        // Perform an update if the key already exists,
        // otherwise insert a new entry.
        bool put(PSName key, const PSObject& value) noexcept 
        {
            if (loadFactor() > 0.75)
                grow();

            size_t slot;
            if (!findSlotForUpsertIn(fEntries, fCapacity, key, slot))
            {
                return false;
            }

            if (fEntries[slot].isEmpty()) {
                fCount++;
            }
            fEntries[slot].key = key;
            fEntries[slot].value = value;

            return true;
        }


        // get
        // Retrieve the value for the key, if it exists.
        bool get(PSName key, PSObject& outValue) const noexcept
        {
            size_t slot;
            
            if (!findKey(key, slot))
                return false;

            outValue = fEntries[slot].value;
            
            return true;
        }

        // remove
        // Remove the entry for the key, if it exists.
        // Creates empty slot, and reduces the count.
        bool remove(PSName key) noexcept
        {
            size_t slot;
            if (!findKey(key, slot))
                return false;

            // clear the slot
            fEntries[slot].key = PSName();
            fEntries[slot].value.reset();

            fCount--;  // adjust count
            return true;
        }

        bool copyEntryFrom(const PSDictionary* other, const PSName& key) 
        {
            if (!other) return false; // no other dictionary

            PSObject value;
            if (!other->get(key, value)) return false;

            put(key, value);
            
            return true;
        }

        // Find the slot of the key.
        // return true if it's found, and slot == the slot
        // return false if it's not found, and slot indicates
        // where it could be placed.
        bool contains(const PSName key) const noexcept
        {
            size_t slot;
            return findKey(key, slot);
        }


        void clear() noexcept
        {
            for (size_t i = 0; i < fCapacity; ++i) {
                fEntries[i].key = PSName();       // invalidate key
                fEntries[i].value.reset();        // drop shared_ptrs, destroy objects
            }
            fCount = 0;
        }

        // Support for iterating over the entries
        // applying a supplied function to each entry.
        // Allows the function to mutate entries, although not really because
        // it's only handed the key/values as value types.
        template <typename Fn>
        void forEach(Fn&& fn) noexcept 
        {
            for (size_t i = 0; i < fCapacity; ++i) {
                if (!fEntries[i].isEmpty()) {
                    if (!fn(fEntries[i].key, fEntries[i].value)) break;
                }
            }
        }

        // This one does not allow mutation of the entries
        template <typename Fn>
        void forEachConst(Fn&& fn) const noexcept {
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

        constexpr float loadFactor() const {
            return static_cast<float>(fCount) / static_cast<float>(fCapacity);
        }

        bool grow() noexcept {
            size_t newCapacity = fCapacity * 2;

            PSDictEntry* newEntries = new (std::nothrow) PSDictEntry[newCapacity]();
            if (!newEntries) {
                return false; // allocation failed
            }

            size_t newCount = 0;

            for (size_t i = 0; i < fCapacity; ++i) {
                if (!fEntries[i].isEmpty()) {
                    size_t slot;
                    bool ok = findSlotForUpsertIn(newEntries, newCapacity, fEntries[i].key, slot);
                    if (!ok) {
                        delete[] newEntries;
                        return false; // rehash failed, should never happen if capacity doubled
                    }
                    newEntries[slot].key = fEntries[i].key;
                    newEntries[slot].value = fEntries[i].value;
                    newCount++;
                }
            }

            delete[] fEntries;
            fEntries = newEntries;
            fCapacity = newCapacity;
            fCount = newCount;

            return true;
        }


        // returns true if the key exists, slot is where it is
        bool findKey(PSName key, size_t& slot) const noexcept {
            size_t hash = reinterpret_cast<size_t>(key.c_str());
            size_t index = hash % fCapacity;
            size_t start = index;   // sentinel for wrapping around

            while (true) {
                if (fEntries[index].isEmpty()) {
                    return false; // key cannot be present
                }
                if (fEntries[index].key == key) {
                    slot = index;
                    return true;
                }
                index = (index + 1) % fCapacity;
                if (index == start) {
                    return false; // wrapped around, not found
                }
            }
        }

        // returns an empty slot for a key to be inserted
        // returns true if a slot was found (always true unless the table is truly full)
        static bool findSlotForUpsertIn(PSDictEntry* entries, size_t cap, PSName key, size_t& slot) noexcept
        {
            size_t hash = reinterpret_cast<size_t>(key.c_str());
            size_t index = hash % cap;
            size_t start = index;

            while (true) {
                if (!entries[index].key.isValid() || entries[index].key == key) {
                    slot = index;
                    return true;
                }
                index = (index + 1) % cap;
                if (index == start) {
                    return false; // table full, should never happen if grow doubled
                }
            }
        }

    };

    /*
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

        bool remove(const PSName& key) {
            return fEntries.remove(key);
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
    */

} // namespace waavs

