#include "ps_lexer.h"
#include "mappedfile.h"
#include "ocspan.h"


using namespace waavs;



void printLexeme(const PSLexeme& lex)
{
	printf("Lexeme Type: %d   Value: %.*s\n", (int)lex.type, (unsigned int)lex.span.size(), lex.span.data());

}

static void test_lexgen(OctetCursor& s)
{
	PSLexemeGenerator gen(s);
	PSLexeme lexeme;
	while(gen.next(lexeme)) 
	{
		printLexeme(lexeme);
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2)
	{
		printf("Usage: tokenizer <.ps file>\n");
		return 1;
	}

	// create an mmap for the specified file
	const char* filename = argv[1];
	auto mapped = MappedFile::create_shared(filename);

	if (nullptr == mapped)
		return 0;


	OctetCursor s(mapped->data(), mapped->size());

	test_lexgen(s);



	mapped->close();

	return 0;
}
