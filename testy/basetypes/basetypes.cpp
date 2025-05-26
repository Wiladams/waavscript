
#include <cstdio>
#include <iostream>

#include <unordered_map>
#include <string>
#include <string_view>


#include "ocspan.h"

using namespace waavs;

struct TransparentHash {
    using is_transparent = void;

    size_t operator()(std::string_view s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
    size_t operator()(const std::string& s) const noexcept {
        return std::hash<std::string>{}(s);
    }
};

struct TransparentEqual {
    using is_transparent = void;

    bool operator()(std::string_view a, std::string_view b) const noexcept {
        return a == b;
    }
    bool operator()(const std::string& a, std::string_view b) const noexcept {
        return a == b;
    }
    bool operator()(std::string_view a, const std::string& b) const noexcept {
        return a == b;
    }
};


static void test_unordered_map()
{
	std::unordered_map<OctetCursor, int> table;

	// OctetCursor objects are implied and created on the fly
	// Then copied into the unordered_map
	table["span1"] = 1; // Insert the span into the table
	table["span2"] = 2; // Insert another span into the table
	table["span3"] = 3; // Insert yet another span into the table


	for (const auto& entry : table) {
		printf("Key: %.*s, Value: %d\n", entry.first.size(), entry.first.data(), entry.second);
	}
}



static void test_map()
{


    std::unordered_map<std::string, const char*, TransparentHash, TransparentEqual> pool;

    std::string_view sv = "test";

    auto it = pool.find(sv);

    if (it == pool.end()) {
        std::cout << "Not found\n";
    }


}

int main(int argc, char *argv[])
{
	test_unordered_map();

	return 0;
}