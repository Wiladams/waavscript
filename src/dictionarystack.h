#pragma once

#include <vector>
#include <memory>

#include "pscore.h"
#include "ps_type_dictionary.h"


namespace waavs {

    struct PSDictionaryStack {
        static constexpr size_t kPermanentDepth = 2;

    private:
        std::vector<PSDictionaryHandle> stack;

    public:
        PSDictionaryStack() {}

        size_t size() const noexcept { return stack.size(); }


        bool push(PSDictionaryHandle dict) {
            stack.push_back(dict);
            return true;
        }

        bool pop() {
            if (stack.size() > kPermanentDepth) // don't remove systemdict or userdict
                stack.pop_back();

            return true;
        }

        PSDictionaryHandle currentdict() const 
        {
            if (stack.empty())
                return nullptr;

            return stack.back();
        }

        bool getFromTop(size_t index, PSDictionaryHandle& out) const noexcept
        {
            if (index >= stack.size())
                return false;

            out = stack[stack.size() - 1 - index];
            return true;
        }


        PSArrayHandle getStack() const {
            PSArrayHandle out = PSArray::create(stack.size());
            for (size_t i = 0; i < stack.size(); ++i) {
                out->append(PSObject::fromDictionary(stack[i]));
            }
            return out;
        }

        void setStack(const std::vector<PSDictionaryHandle>& newStack)
        {
            stack = newStack;
        }


        bool define(const PSName &key, const PSObject& value) 
        {
            if (stack.empty())
                return false;

			return currentdict()->put(key, value);
        }

        bool load(const PSName &key, PSObject& out) const {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                if ((*it)->get(key, out))
                    return true;
            }
            return false;
        }

        // store
        // 
        // Returns true if the key was found and updated
        // Only updates existing values.  If the key is not found 
        // in any dictionary, return false.
        bool store(const PSName &key, const PSObject& value) const noexcept {
            PSDictionaryHandle out;
            if (!where(key, out))
            {
                //printf("PSDictionaryStack::store: key '%s' not found in any dictionary\n", key.c_str());
                //return false; // key not found
                out = currentdict(); // fallback to current dict
            }
            out->put(key, value);

            return true;
        }

        bool clear() 
        {
            // traverse the stack in reverse order, popping entries
            // We leave 2 (systemdict, userdict) on the stack, but clear all other dictionaries
            if (stack.size() < kPermanentDepth)
                return false;


            while (stack.size() > kPermanentDepth) {
                stack.pop_back();
            }

            return true;
        }

        bool where(const PSName& key, PSDictionaryHandle& outDict) const {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                if ((*it)->contains(key)) {
                    outDict = *it;
                    return true;
                }
            }
            return false;
        }

        template <typename Fn>
        void forEachFromTop(Fn&& fn) const
        {
            for (size_t i = stack.size(); i-- > 0; )
            {
                if (!fn(stack[i])) break;
            }
        }


    };
}
