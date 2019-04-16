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

		void error(std::string msg, const char* file, int line) const
		{
			error(context->locat, msg, file, line);
		}

		void error(const location& arg_loc, std::string msg, const char* file, int line) const
		{
			std::cout << EndL;
			std::cout << "# FILE   : " << file << EndL;
			std::cout << "# LINE   : " << line << EndL;
			std::cout << "# SOURCE : " << arg_loc << EndL;
			std::cout << "# MESSAGE: " << msg << EndL;
			std::cout << EndL;
		}
	};
}

