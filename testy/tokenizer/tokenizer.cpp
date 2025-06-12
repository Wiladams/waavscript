#include "ps_lexer.h"
#include "mappedfile.h"
#include "ocspan.h"


using namespace waavs;



void printLexeme(const PSLexeme& lex)
{
	printf("Lexeme Type: %d   VALUE: %.*s\n", (int)lex.type, (unsigned int)lex.span.size(), lex.span.data());

}

static void test_lexgen(OctetCursor s)
{
	PSLexemeGenerator gen(s);
	PSLexeme lexeme;
	while(gen.next(lexeme)) 
	{
		printLexeme(lexeme);
	}
}

static void test_lexgenfile(const char *filename)
{
	auto mapped = MappedFile::create_shared(filename);
	if (nullptr == mapped)
		return;
	OctetCursor s(mapped->data(), mapped->size());
	test_lexgen(s);
	mapped->close();
}

int main(int argc, char *argv[]) {
	if (argc >= 2)
	{
		test_lexgenfile(argv[1]);
		return 0;
	}

	test_lexgen("/x 42 def x =");

	return 0;
}
