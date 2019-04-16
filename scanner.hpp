#pragma once

#include "parser.hh"
#include "driver.hpp"

#if ! defined(yyFlexLexerOnce)
	#include <FlexLexer.h>
#endif

namespace Sql1
{
	class Scanner : public yyFlexLexer
	{
	public:

/*
		void user_action(location& arg_loc)
		{
			arg_loc.columns(YYLeng());
		}

		void user_init(location& arg_loc)
		{
			int iii = 0;
			iii++;
		}
*/
		std::string temp_str;

	public:
		Parser::symbol_type Scan_(Driver& driver);
		Parser::symbol_type Scan(Driver& driver);
	};
}

