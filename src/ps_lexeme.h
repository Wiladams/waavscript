// ps_lexeme.h
#pragma once

#include "ocspan.h"

namespace waavs
{
	enum class PSLexType {
		Invalid = 0,
        Whitespace,		// space, tab, newline, carriage return - skip
		Name,			// name without leading /, e.g. moveto
		LiteralName,	// /name with leading /, e.g. /moveto
		ImmediateName,		// //moveto
		Number,
		String,
		UnterminatedString,
		HexString,
		ASCII85String,
        LBRACE,			// {	- begin procedure marker
		RBRACE,			// }	- end procedure marker
        LBRACKET,		// [	- begin array marker
        RBRACKET,		// ]	- end array marker
		LLANGLE,		// for <<
		RRANGLE,		// for >>
		Comment,
		DSCComment,		// %%DSCKeyword value
		Delimiter,
		Eof
	};

	struct PSLexeme
	{
		PSLexType type = PSLexType::Invalid;
		OctetCursor span; // points to original data slice
	};
}