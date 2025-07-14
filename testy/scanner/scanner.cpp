
#include "mappedfile.h"

#include "ps_scanner.h"
#include "ps_print.h"


using namespace waavs;


static void test_objectgen(const OctetCursor& s)
{
	PSObjectGenerator gen(s);
	PSObject obj;
	while (gen.next(obj))
	{
        writeObjectDeep(obj, std::cout);
        printf("\n");
	}
}

static void test_objectgenFile(const char* filename)
{
	// create an mmap for the specified file
	auto mapped = MappedFile::create_shared(filename);

	if (nullptr == mapped)
		return ;


	OctetCursor s(mapped->data(), mapped->size());

	test_objectgen(s);

	mapped->close();
}


int main(int argc, char* argv[]) {
	if (argc >= 2)
	{
		test_objectgenFile(argv[1]);
		return 1;
	}

	test_objectgen(OctetCursor("/x 42 def x ="));

	return 0;
}

