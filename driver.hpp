#pragma once

#include <string>

#include "parser.hh"
#include "comm.hpp"

namespace Sql1
{
	class Context
	{
	public:
		std::string filename;
		const MainConfig* config;
		location locat;

		explicit Context(const std::string& arg_filename, const MainConfig* arg_config)
			: filename{ arg_filename }, config{ arg_config }, locat{ &filename } { }
	};

	class Driver
	{
	public:
		Context* context;

		explicit Driver(Context* arg_context) : context{ arg_context } { }

		void error(std::ostream& os, std::string msg, const char* file, int line) const
		{
			error(os, context->locat, msg, file, line);
		}

		void error(std::ostream& os, const location& arg_loc, std::string msg,
			const char* file, int line) const
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

