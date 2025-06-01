
#include <cstdio>
#include <iostream>

#include <unordered_map>
#include <map>
#include <string>
#include <string_view>


#include "ocspan.h"
#include "pscore.h"
#include "psstack.h"
#include "nametable.h"

using namespace waavs;


static void test_nametable()
{
	printf("Testing NameTable interned strings...\n");
	// Create a NameTable and add some names to it
	const char* literal1 = PSNameTable::INTERN(OctetCursor("leteral1"));
	const char* literal2 = PSNameTable::INTERN("literal2");
	const char* literal3 = PSNameTable::INTERN("literal3");

	const char* literal11 = PSNameTable::INTERN("leteral1");
	const char* literal21 = PSNameTable::INTERN("literal2");
	const char* literal31 = PSNameTable::INTERN("literal3");

	printf(" literal1: %p\n", literal1);
	printf("literal11: %p\n", literal11);

	printf(" literal2: %p\n", literal2);
	printf("literal21: %p\n", literal21);

	printf(" literal3: %p\n", literal3);
	printf("literal31: %p\n", literal31);

}

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

static void test_bool()
{
	PSObject obj = PSObject::fromBool(false);
	printf("bool stored: %d\n", obj.asBool());  // Expect 0
}

static void test_psobject()
{
	PSObjectStack stack1;
	PSObjectStack stack2;

	// create a PSString based onject, put it onto the stack
	// then pop it off
	auto str = PSString::createFromCString("Hello, World!");
	PSObject obj;
	for (int i = 0; i < 10; ++i) {
		obj.resetFromString(str);
		stack1.push(obj);

		stack2.push(obj);  // Push the same object onto another stack
	}

	printf("STACK 2\n");
	PSObject obj2;
	while (!stack2.empty()) {
		stack2.pop(obj2);
		std::cout << obj2.asString()->toString() << std::endl;  // Expect "Hello, World!"
	}

	printf("STACK 1\n");
	// Now try to do the same with the first stack
	while (!stack1.empty()) {
		stack1.pop(obj);
		std::cout << obj.asString()->toString() << std::endl;  // Expect "Hello, World!"
	}

}


int main(int argc, char *argv[])
{
	//test_unordered_map();
	//test_bool();
	//test_psobject();
	test_nametable();

	return 0;
}