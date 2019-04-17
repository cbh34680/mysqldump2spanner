#pragma once

#include <string>

#include "context.hpp"
#include "parser.hh"

namespace Sql1
{
	class Driver
	{
	public:
		Context* context;

		explicit Driver(Context* arg_context) : context{ arg_context } { }

		void error(std::ostream& os, std::string msg, const char* file, int line) const
		{
			error(os, context->locat, msg, file, line);
		}

		void error(std::ostream& os, const location& arg_loc, std::string msg, const char* file, int line) const
		{
			os << EndL;
			os << "# FILE   : " << file << EndL;
			os << "# LINE   : " << line << EndL;
			os << "# SOURCE : " << arg_loc << EndL;
			os << "# MESSAGE: " << msg << EndL;
			os << EndL;
		}
	};
}

