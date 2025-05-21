#pragma once

#include <vector>
#include <memory>
#include "pscore.h"

namespace waavsps {
    struct PSDictionaryStack {
        std::vector<std::shared_ptr<PSDictionary>> stack;

        PSDictionaryStack() {
            // Push global dictionary
            stack.push_back(std::make_shared<PSDictionary>());
        }

        void push(std::shared_ptr<PSDictionary> dict) {
            stack.push_back(dict);
        }

        void pop() {
            if (stack.size() > 1) // don't remove global
                stack.pop_back();
        }

        std::shared_ptr<PSDictionary> currentdict() {
            return stack.back();
        }

        bool def(const std::string& key, const PSObject& value) {
            return currentdict()->put(key, value);
        }

        bool load(const std::string& key, PSObject& out) const {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                if ((*it)->get(key, out))
                    return true;
            }
            return false;
        }

        bool store(const std::string& key, const PSObject& value) {
            for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
                if ((*it)->contains(key)) {
                    (*it)->put(key, value);
                    return true;
                }
            }
            return def(key, value); // fallback to current
        }

        void clear() {
            stack.clear();
            stack.push_back(std::make_shared<PSDictionary>()); // restore global
        }
    };
}
