#pragma once

namespace Sql1
{
	class Context
	{
	public:
		std::string filename;
		location locat;

		explicit Context(const std::string& arg_filename) : filename{ arg_filename }, locat{ &filename } { }
	};
}

