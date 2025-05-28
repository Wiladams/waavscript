
#include <cstdio>
#include <iostream>

#include <unordered_map>
#include <map>
#include <string>
#include <string_view>


#include "ocspan.h"

using namespace waavs;




static void test_unordered_map()
{
	std::unordered_map<OctetCursor, int> table;

	// OctetCursor objects are implied and created on the fly
	// Then copied into the unordered_map
	table["span1"] = 1; // Insert the span into the table
	table["span2"] = 2; // Insert another span into the table
	table["span3"] = 3; // Insert yet another span into the table


	for (const auto& entry : table) {
		printf("Key: %.*s, Value: %d\n", (int)entry.first.size(), entry.first.data(), entry.second);
	}
}





int main(int argc, char *argv[])
{
	test_unordered_map();

	return 0;
}