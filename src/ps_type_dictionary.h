#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <new>

#include "pscore.h"
#include "ps_type_name.h"

namespace waavs {

    struct PSDictEntry
    {
        PSObject key;
        PSObject value;
        bool occupied{ false };

        bool isEmpty() const noexcept
        {
            return !occupied;
        }

        void reset() noexcept
        {
            key.reset();
            value.reset();
            occupied = false;
        }
    };


    class PSDictionary
    {
        PSDictionary() = delete;

    public:
        explicit PSDictionary(size_t initialCapacity)
            : fCapacity(std::max(size_t(4), initialCapacity))
            , fCount(0)
        {
            fEntries = new PSDictEntry[fCapacity]();
        }

        ~PSDictionary()
        {
            delete[] fEntries;
        }

        PSDictionary(const PSDictionary&) = delete;
        PSDictionary& operator=(const PSDictionary&) = delete;
        PSDictionary(PSDictionary&&) = delete;
        PSDictionary& operator=(PSDictionary&&) = delete;


        static std::shared_ptr<PSDictionary> create(size_t initialSize = 32)
        {
            return std::shared_ptr<PSDictionary>(new PSDictionary(initialSize));
        }


        constexpr size_t size() const noexcept
        {
            return fCount;
        }

        constexpr bool empty() const noexcept
        {
            return fCount == 0;
        }


        // ------------------------------------------------------------
        // General PSObject-key API
        // ------------------------------------------------------------

        bool put(const PSObject& inputKey, const PSObject& value)
        {
            PSObject key;
            if (!normalizeKey_(inputKey, key))
                return false;

            size_t slot;
            if (!findSlotForUpsertIn(fEntries, fCapacity, key, slot))
                return false;

            // Only grow for a new insertion. Updating an existing key
            // does not increase the load factor.
            if (fEntries[slot].isEmpty() && ((fCount + 1) * 4 > fCapacity * 3))
            {
                if (!grow())
                    return false;

                if (!findSlotForUpsertIn(fEntries, fCapacity, key, slot))
                    return false;
            }

            if (fEntries[slot].isEmpty())
            {
                fEntries[slot].key = key;
                fEntries[slot].occupied = true;
                ++fCount;
            }

            fEntries[slot].value = value;
            return true;
        }


        bool get(const PSObject& inputKey, PSObject& outValue) const
        {
            PSObject key;
            if (!normalizeKey_(inputKey, key))
                return false;

            size_t slot;
            if (!findKey(key, slot))
                return false;

            outValue = fEntries[slot].value;
            return true;
        }


        bool remove(const PSObject& inputKey)
        {
            PSObject key;
            if (!normalizeKey_(inputKey, key))
                return false;

            size_t slot;
            if (!findKey(key, slot))
                return false;

            fEntries[slot].reset();
            --fCount;

            // Repair the open-addressing cluster following the removed slot.
            size_t index = (slot + 1) % fCapacity;

            while (!fEntries[index].isEmpty())
            {
                PSDictEntry entry = std::move(fEntries[index]);

                fEntries[index].reset();
                --fCount;

                size_t newSlot;
                if (!findSlotForUpsertIn(fEntries, fCapacity, entry.key, newSlot))
                    return false;

                fEntries[newSlot] = std::move(entry);
                ++fCount;

                index = (index + 1) % fCapacity;
            }

            return true;
        }


        bool contains(const PSObject& inputKey) const
        {
            PSObject key;
            if (!normalizeKey_(inputKey, key))
                return false;

            size_t slot;
            return findKey(key, slot);
        }


        bool copyEntryFrom(const PSDictionary* other, const PSObject& key)
        {
            if (!other)
                return false;

            PSObject value;
            if (!other->get(key, value))
                return false;

            return put(key, value);
        }


        // ------------------------------------------------------------
        // PSName convenience API
        //
        // Keep these so the overwhelmingly common name-keyed code does
        // not have to manufacture PSObjects at every call site.
        // ------------------------------------------------------------

        bool put(const PSName& key, const PSObject& value)
        {
            return put(PSObject::fromName(key), value);
        }

        bool get(const PSName& key, PSObject& outValue) const
        {
            return get(PSObject::fromName(key), outValue);
        }

        bool remove(const PSName& key)
        {
            return remove(PSObject::fromName(key));
        }

        bool contains(const PSName& key) const
        {
            return contains(PSObject::fromName(key));
        }

        bool copyEntryFrom(const PSDictionary* other, const PSName& key)
        {
            return copyEntryFrom(other, PSObject::fromName(key));
        }


        // ------------------------------------------------------------
        // Management / iteration
        // ------------------------------------------------------------

        void clear() noexcept
        {
            for (size_t i = 0; i < fCapacity; ++i)
                fEntries[i].reset();

            fCount = 0;
        }


        bool nextEntry(size_t& cursor, PSObject& key, PSObject& value) const noexcept
        {
            while (cursor < fCapacity)
            {
                const PSDictEntry& entry = fEntries[cursor++];

                if (entry.isEmpty())
                    continue;

                key = entry.key;
                value = entry.value;
                return true;
            }

            return false;
        }


        // Compatibility helper for dictionaries known to contain only
        // name keys, such as resource dictionaries.
        bool nextEntry(size_t& cursor, PSName& key, PSObject& value) const noexcept
        {
            while (cursor < fCapacity)
            {
                const PSDictEntry& entry = fEntries[cursor++];

                if (entry.isEmpty())
                    continue;

                if (!entry.key.isName())
                    return false;

                key = entry.key.asName();
                value = entry.value;
                return true;
            }

            return false;
        }


        template <typename Fn>
        void forEach(Fn&& fn)
        {
            for (size_t i = 0; i < fCapacity; ++i)
            {
                PSDictEntry& entry = fEntries[i];

                if (entry.isEmpty())
                    continue;

                const PSObject& key = entry.key;
                PSObject& value = entry.value;

                if (!fn(key, value))
                    break;
            }
        }


        template <typename Fn>
        void forEachConst(Fn&& fn) const
        {
            for (size_t i = 0; i < fCapacity; ++i)
            {
                if (fEntries[i].isEmpty())
                    continue;

                if (!fn(fEntries[i].key, fEntries[i].value))
                    break;
            }
        }


    private:
        PSDictEntry* fEntries{ nullptr };
        size_t fCapacity{ 0 };
        size_t fCount{ 0 };


        // ------------------------------------------------------------
        // Key normalization
        //
        // Strings used as dictionary keys are converted to names.
        // Names are canonicalized as literal names so executable state
        // does not participate in key identity.
        // ------------------------------------------------------------

        static bool normalizeKey_(const PSObject& input, PSObject& output)
        {
            switch (input.type)
            {
            case PSObjectType::Null:
            case PSObjectType::Invalid:
            case PSObjectType::Any:
                return false;

            case PSObjectType::String:
            {
                const PSString& str = input.asString();

                OctetCursor span(str.data(), str.length());
                PSName name(PSNameTable::INTERN(span));

                output = PSObject::fromName(name);
                return true;
            }

            case PSObjectType::Name:
                output = PSObject::fromName(input.asName());
                return true;

            case PSObjectType::Real:
                if (!std::isfinite(input.asReal()))
                    return false;

                output = input;
                return true;

            case PSObjectType::Float:
                if (!std::isfinite(static_cast<double>(input.as<float>())))
                    return false;

                output = input;
                return true;

            default:
                output = input;
                return isSupportedKey_(output);
            }
        }


        static bool isSupportedKey_(const PSObject& key) noexcept
        {
            switch (key.type)
            {
            case PSObjectType::Int:
            case PSObjectType::Float:
            case PSObjectType::Real:
            case PSObjectType::Bool:
            case PSObjectType::Name:
            case PSObjectType::Pointer:
            case PSObjectType::Array:
            case PSObjectType::Dictionary:
            case PSObjectType::File:
            case PSObjectType::Font:
            case PSObjectType::FontFace:
            case PSObjectType::Operator:
            case PSObjectType::Mark:
                return true;

                // These are internal/value representations for which the VM
                // currently has no suitable PostScript identity semantics.
            case PSObjectType::Matrix:
            case PSObjectType::Path:
            case PSObjectType::Save:
            case PSObjectType::String:
            case PSObjectType::Null:
            case PSObjectType::Invalid:
            case PSObjectType::Any:
            default:
                return false;
            }
        }


        // ------------------------------------------------------------
        // Key equality
        // ------------------------------------------------------------

        static bool isNumericType_(PSObjectType type) noexcept
        {
            return type == PSObjectType::Int ||
                type == PSObjectType::Float ||
                type == PSObjectType::Real;
        }


        static double numericValue_(const PSObject& key) noexcept
        {
            switch (key.type)
            {
            case PSObjectType::Int:
                return static_cast<double>(key.as<int32_t>());

            case PSObjectType::Float:
                return static_cast<double>(key.as<float>());

            case PSObjectType::Real:
                return key.as<double>();

            default:
                return 0.0;
            }
        }


        static bool keyEqual_(const PSObject& a, const PSObject& b) noexcept
        {
            if (isNumericType_(a.type) && isNumericType_(b.type))
                return numericValue_(a) == numericValue_(b);

            if (a.type != b.type)
                return false;

            switch (a.type)
            {
            case PSObjectType::Bool:
                return a.asBool() == b.asBool();

            case PSObjectType::Name:
                return a.asName() == b.asName();

            case PSObjectType::Pointer:
                return a.as<void*>() == b.as<void*>();

            case PSObjectType::Array:
                return a.asArray().get() == b.asArray().get();

            case PSObjectType::Dictionary:
                return a.asDictionary().get() == b.asDictionary().get();

            case PSObjectType::File:
                return a.asFile().get() == b.asFile().get();

            case PSObjectType::Font:
                return a.asFont().get() == b.asFont().get();

            case PSObjectType::FontFace:
                return a.asFontFace().get() == b.asFontFace().get();

            case PSObjectType::Operator:
                return a.asOperator().name() == b.asOperator().name();

            case PSObjectType::Mark:
                return a.asMark().name() == b.asMark().name();

            default:
                return false;
            }
        }


        // ------------------------------------------------------------
        // Key hashing
        //
        // Anything that keyEqual_ considers equal MUST hash identically.
        // In particular Int/Float/Real all use the same numeric hash.
        // ------------------------------------------------------------

        static size_t hashCombine_(size_t tag, size_t value) noexcept
        {
            constexpr size_t k =
                sizeof(size_t) == 8
                ? static_cast<size_t>(0x9e3779b97f4a7c15ull)
                : static_cast<size_t>(0x9e3779b9u);

            return value ^ (tag + k + (value << 6) + (value >> 2));
        }


        static size_t hashPointer_(const void* ptr) noexcept
        {
            return std::hash<const void*>{}(ptr);
        }


        static size_t hashKey_(const PSObject& key) noexcept
        {
            if (isNumericType_(key.type))
                return hashCombine_(1, std::hash<double>{}(numericValue_(key)));

            switch (key.type)
            {
            case PSObjectType::Bool:
                return hashCombine_(2, std::hash<bool>{}(key.asBool()));

            case PSObjectType::Name:
                return hashCombine_(3, hashPointer_(key.asName().c_str()));

            case PSObjectType::Pointer:
                return hashCombine_(4, hashPointer_(key.as<void*>()));

            case PSObjectType::Array:
                return hashCombine_(5, hashPointer_(key.asArray().get()));

            case PSObjectType::Dictionary:
                return hashCombine_(6, hashPointer_(key.asDictionary().get()));

            case PSObjectType::File:
                return hashCombine_(7, hashPointer_(key.asFile().get()));

            case PSObjectType::Font:
                return hashCombine_(8, hashPointer_(key.asFont().get()));

            case PSObjectType::FontFace:
                return hashCombine_(9, hashPointer_(key.asFontFace().get()));

            case PSObjectType::Operator:
                return hashCombine_(10, hashPointer_(key.asOperator().name().c_str()));

            case PSObjectType::Mark:
                return hashCombine_(11, hashPointer_(key.asMark().name().c_str()));

            default:
                return 0;
            }
        }


        // ------------------------------------------------------------
        // Open-addressing implementation
        // ------------------------------------------------------------

        bool grow()
        {
            const size_t newCapacity = fCapacity * 2;

            PSDictEntry* newEntries = new (std::nothrow) PSDictEntry[newCapacity]();
            if (!newEntries)
                return false;

            size_t newCount = 0;

            for (size_t i = 0; i < fCapacity; ++i)
            {
                if (fEntries[i].isEmpty())
                    continue;

                size_t slot;
                if (!findSlotForUpsertIn(newEntries, newCapacity, fEntries[i].key, slot))
                {
                    delete[] newEntries;
                    return false;
                }

                newEntries[slot].key = fEntries[i].key;
                newEntries[slot].value = fEntries[i].value;
                newEntries[slot].occupied = true;
                ++newCount;
            }

            delete[] fEntries;

            fEntries = newEntries;
            fCapacity = newCapacity;
            fCount = newCount;

            return true;
        }


        bool findKey(const PSObject& key, size_t& slot) const noexcept
        {
            size_t index = hashKey_(key) % fCapacity;
            const size_t start = index;

            while (true)
            {
                const PSDictEntry& entry = fEntries[index];

                if (entry.isEmpty())
                    return false;

                if (keyEqual_(entry.key, key))
                {
                    slot = index;
                    return true;
                }

                index = (index + 1) % fCapacity;

                if (index == start)
                    return false;
            }
        }


        static bool findSlotForUpsertIn(PSDictEntry* entries, size_t capacity, const PSObject& key, size_t& slot) noexcept
        {
            size_t index = hashKey_(key) % capacity;
            const size_t start = index;

            while (true)
            {
                PSDictEntry& entry = entries[index];

                if (entry.isEmpty() || keyEqual_(entry.key, key))
                {
                    slot = index;
                    return true;
                }

                index = (index + 1) % capacity;

                if (index == start)
                    return false;
            }
        }
    };

} // namespace waavs