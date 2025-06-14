#pragma once

#include <vector>
#include <memory>

#include "pscore.h"

namespace waavs {
    struct PSDictionaryStack {
        std::vector<PSDictionaryHandle> stack;

        PSDictionaryStack() {
            // Push global dictionary
            stack.push_back(PSDictionary::create());
        }

        void push(PSDictionaryHandle dict) {
            stack.push_back(dict);
        }

        void pop() {
            if (stack.size() > 1) // don't remove global
                stack.pop_back();
        }

        PSDictionaryHandle currentdict() {
            return stack.back();
        }

        bool def(const char * key, const PSObject& value) 
        {
			auto dict = currentdict();
            return dict->put(key, value);
        }

        bool load(const char * key, PSObject& out) const {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                if ((*it)->get(key, out))
                    return true;
            }
            return false;
        }

        bool store(const char * key, const PSObject& value) {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) 
            {
                if ((*it)->contains(key)) {
                    (*it)->put(key, value);
                    return true;
                }
            }
            return def(key, value); // fallback to current
        }

        void clear() {
            stack.clear();
            stack.push_back(PSDictionary::create()); // restore global
        }
    };
}
