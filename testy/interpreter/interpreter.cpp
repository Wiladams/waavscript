
#include "mappedfile.h"


#include "psvmfactory.h"
#include "ps_interpreter.h"


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

	PSInterpreter interp(*vm);
	interp.interpret(src);


}

void runPostScript(const char* sourceText) {
	OctetCursor input(sourceText);
	runInterpreter(input);
}


static void test_interpreter()
{
	runPostScript("1 2 add dup =");
}


int main(int argc, char* argv[]) 
{
	test_interpreter();

	
	if (argc < 2)
	{
		printf("\nUsage: interpreter <.ps file>\n");
		return 1;
	}

	// create an mmap for the specified file
	const char* filename = argv[1];
	auto mapped = MappedFile::create_shared(filename);

	if (nullptr == mapped)
		return 0;


	OctetCursor s(mapped->data(), mapped->size());

	runInterpreter(s);

	mapped->close();

	return 0;
}


