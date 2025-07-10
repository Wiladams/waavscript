#include "ocspan.h"
#include "ps_lexer.h"
#include "c_preproc.h"
#include "ps_print.h"

using namespace waavs;

void test_lexer()
{
	const char* decl = "int add(int a, int b);";
	OctetCursor src(decl);
	PrepToken tok;

	while (nextPreprocToken(src, tok))
	{
		printf("Token: %d  Value: %.*s\n", tok.type, tok.span.size(), tok.span.data());
	}

	auto dict = PSDictionary::create();
}


void test_parser() {
	const char* decl = "int add(int a, int b);";
	OctetCursor src(decl);
	auto dict = PSDictionary::create();

	if (parseFunctionDeclaration(src, *dict)) {
		writeDictDeep(std::cout, dict);
	}

}

void test_lexer_file(const char* filename)
{
}

int main(int argc, char* argv[]) {

	// We have a file to test
	if (argc >= 2)
	{
		test_lexer_file(argv[1]);
		return 0;
	}

	// No file, just test some cases directly
	test_parser();
	//test_lexer();

	return 0;
}
