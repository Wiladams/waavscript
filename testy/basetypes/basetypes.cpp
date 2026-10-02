
#include <cstdio>
#include <iostream>

#include <unordered_map>
#include <map>
#include <string>
#include <string_view>


#include "ocspan.h"
#include "pscore.h"
#include "ps_type_stack.h"
#include "ps_type_dictionary.h"


using namespace waavs;


static void test_nametable()
{
	printf("Testing NameTable interned strings...\n");
	// Create a NameTable and add some names to it
	const char* literal1 = PSNameTable::INTERN(OctetCursor("literal1"));
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
	auto str = PSString::fromCString("Hello, World!");
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
		std::cout << obj2.asString().toString() << std::endl;  // Expect "Hello, World!"
	}

	printf("STACK 1\n");
	// Now try to do the same with the first stack
	while (!stack1.empty()) {
		stack1.pop(obj);
		std::cout << obj.asString().toString() << std::endl;  // Expect "Hello, World!"
	}

}

static void test_psdictionary_remove_probe_chain()
{
	printf("Testing PSDictionary remove() probe-chain preservation...\n");

	auto dict = PSDictionary::create(4);

	// We want several names that land in the same initial hash bucket.
	// Since PSDictionary hashes the interned string pointer, search for a collision.
	PSName names[3];
	size_t found = 0;
	size_t targetBucket = 0;

	for (int i = 0; found < 3 && i < 10000; ++i)
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "collision_name_%d", i);

		PSName name(PSNameTable::INTERN(buffer));
		size_t bucket = reinterpret_cast<size_t>(name.c_str()) % 4;

		if (found == 0) {
			targetBucket = bucket;
			names[found++] = name;
		}
		else if (bucket == targetBucket) {
			names[found++] = name;
		}
	}

	if (found < 3) {
		printf("FAIL: could not find enough colliding names\n");
		return;
	}

	dict->put(names[0], PSObject::fromInt(10));
	dict->put(names[1], PSObject::fromInt(20));
	dict->put(names[2], PSObject::fromInt(30));

	PSObject value;

	if (!dict->get(names[0], value) || value.asInt() != 10)
		printf("FAIL: first colliding key missing before remove\n");

	if (!dict->get(names[1], value) || value.asInt() != 20)
		printf("FAIL: second colliding key missing before remove\n");

	if (!dict->get(names[2], value) || value.asInt() != 30)
		printf("FAIL: third colliding key missing before remove\n");

	if (!dict->remove(names[0]))
		printf("FAIL: remove() failed\n");

	if (dict->get(names[0], value))
		printf("FAIL: removed key still present\n");

	if (!dict->get(names[1], value) || value.asInt() != 20)
		printf("FAIL: second key lost after removing earlier probe-chain entry\n");

	if (!dict->get(names[2], value) || value.asInt() != 30)
		printf("FAIL: third key lost after removing earlier probe-chain entry\n");

	printf("PSDictionary remove() probe-chain test complete\n");
}


static void test_psdictionary_next_entry()
{
	printf("Testing PSDictionary nextEntry()...\n");

	auto dict = PSDictionary::create(8);

	PSName nameA("alpha");
	PSName nameB("beta");
	PSName nameC("gamma");

	dict->put(nameA, PSObject::fromInt(10));
	dict->put(nameB, PSObject::fromInt(20));
	dict->put(nameC, PSObject::fromInt(30));

	size_t cursor = 0;
	PSName key;
	PSObject value;

	bool foundA = false;
	bool foundB = false;
	bool foundC = false;
	size_t count = 0;

	while (dict->nextEntry(cursor, key, value))
	{
		count++;

		if (key == nameA) {
			foundA = value.isInt() && value.asInt() == 10;
		}
		else if (key == nameB) {
			foundB = value.isInt() && value.asInt() == 20;
		}
		else if (key == nameC) {
			foundC = value.isInt() && value.asInt() == 30;
		}
		else {
			printf("FAIL: unexpected dictionary key during iteration: %s\n", key.c_str());
		}
	}

	if (count != 3)
		printf("FAIL: expected 3 dictionary entries, got %zu\n", count);

	if (!foundA)
		printf("FAIL: alpha not found during iteration\n");

	if (!foundB)
		printf("FAIL: beta not found during iteration\n");

	if (!foundC)
		printf("FAIL: gamma not found during iteration\n");

	printf("PSDictionary nextEntry() test complete\n");
}


int main(int argc, char *argv[])
{
	//test_unordered_map();
	//test_bool();
	//test_psobject();
	//test_nametable();
	test_psdictionary_remove_probe_chain();
	test_psdictionary_next_entry();

	return 0;
}