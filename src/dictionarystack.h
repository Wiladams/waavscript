#pragma once

#include <vector>
#include <memory>

#include "pscore.h"
#include "psdictionary.h"


namespace waavs {
    struct PSDictionaryStack {
    private:
        std::vector<PSDictionaryHandle> stack;

    public:
        PSDictionaryStack() {}

        bool push(PSDictionaryHandle dict) {
            stack.push_back(dict);
            return true;
        }

        bool pop() {
            if (stack.size() > 1) // don't remove global
                stack.pop_back();

            return true;
        }

        PSDictionaryHandle currentdict() const {
            return stack.back();
        }

        bool getCount(size_t& count) const {
            count = stack.size();
            return true;
        }

        PSArrayHandle getStack() const {
            PSArrayHandle out = PSArray::create(stack.size());
            for (size_t i = 0; i < stack.size(); ++i) {
                out->append(PSObject::fromDictionary(stack[i]));
            }
            return out;
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

        bool clear() {
            // traverse the stack in reverse order, popping entries
            // leaving the gobal dictionary intact

            if (stack.empty())
                return false;

            while (stack.size() > 1) {
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

    };
}
