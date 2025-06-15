
#include "mappedfile.h"


#include "psvmfactory.h"


using namespace waavs;



static void runInterpreter(OctetCursor& src)
{
	// get a virtual machine
	std::unique_ptr<PSVirtualMachine> vm = PSVMFactory::createVM();
	if (nullptr == vm)
	{
		printf("Failed to create virtual machine\n");
		return;
	}

	vm->interpret(src);
}

void runPostScript(const char* sourceText) {
	OctetCursor input(sourceText);
	runInterpreter(input);
}


static void test_interpreter()
{
	//runPostScript("1 2 add dup =");
	runPostScript("/x 42 def x =");
	runPostScript("/x {dup mul} def 3 x =");
}

static int test_interpreterFile(const char* filename)
{
	// create an mmap for the specified file
	auto mapped = MappedFile::create_shared(filename);
	if (nullptr == mapped)
		return 0;
	OctetCursor s(mapped->data(), mapped->size());
	runInterpreter(s);
	mapped->close();

	return 0;
}

int main(int argc, char* argv[]) 
{
	
	if (argc >= 2)
	{
		return test_interpreterFile(argv[1]);

	}

	test_interpreter();

	return 0;
}


