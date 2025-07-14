#include <unordered_map>

#include "ps_lexer.h"
#include "mappedfile.h"
#include "ocspan.h"


using namespace waavs;

/*
	enum class PSLexType {
		Invalid = 0,
		Whitespace,
		Name,			// name without leading /, e.g. moveto
		LiteralName,	// /name with leading /, e.g. /moveto
		SystemName,		// //moveto
		Number,
		String,
		UnterminatedString,
		HexString,
		LBRACE,			// {
		RBRACE,			// }
		LBRACKET,		// [
		RBRACKET,		// ]
		LLANGLE,		// for <<
		RRANGLE,		// for >>
		Comment,
		DSCComment,		// %%DSCKeyword value
		Delimiter,
		EexecSwitch,	// eexec switch
		Eof
	};

*/

std::unordered_map<PSLexType, const char*> lexTypeNames = {
	{PSLexType::Invalid, "Invalid"},
	{PSLexType::Whitespace, "Whitespace"},
	{PSLexType::Name, "Name"},
	{PSLexType::LiteralName, "LiteralName"},
	{PSLexType::SystemName, "SystemName"},
	{PSLexType::Number, "Number"},
	{PSLexType::String, "String"},
	{PSLexType::UnterminatedString, "UnterminatedString"},
	{PSLexType::HexString, "HexString"},
	{PSLexType::LBRACE, "LBRACE"},
	{PSLexType::RBRACE, "RBRACE"},
	{PSLexType::LBRACKET, "LBRACKET"},
	{PSLexType::RBRACKET, "RBRACKET"},
	{PSLexType::LLANGLE, "LLANGLE"},
	{PSLexType::RRANGLE, "RRANGLE"},
	{PSLexType::Comment, "Comment"},
	{PSLexType::DSCComment, "DSCComment"},
	{PSLexType::Delimiter, "Delimiter"},
    {PSLexType::EexecSwitch, "EexecSwitch"},
	{PSLexType::Eof, "Eof"}
};

void printLexeme(const PSLexeme& lex)
{
    const char* typeName = "UNKNOWN";
    auto it = lexTypeNames.find(lex.type);
	if (it != lexTypeNames.end()) {
		typeName = it->second;
    }
	printf("Lexeme Type: %2d  %16s  VALUE: %.*s\n", (int)lex.type, typeName, (unsigned int)lex.span.size(), lex.span.data());
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
