#pragma once

#include <memory>
#include <vector>

namespace Sql1
{
	class Insert : public Stmt
	{
	public:
		explicit Insert(const std::string& name, Strings&& colnames)
			: mName{ name }, mColnames{ std::move(colnames) } { }

		void output(std::ostream& os) const override
		{
			os << "dml-type=[insert] table=[" << mName << "]" << std::endl << std::endl;

			os << indent_manip::push;

				os << "* COLUMN" << std::endl;
				os << indent_manip::push;
				os << "names=[" << mColnames << "]" << std::endl;
				os << indent_manip::pop;

			os << indent_manip::pop;
		}

		std::string convert() const override
		{
			std::string ret{ "INSERT INTO " };

			ret += mName + " (";
			ret += join_strs(mColnames, ",");
			ret += ")";

			return std::move(ret);
		}

	private:
		std::string mName;
		Strings mColnames;
	};

	using InsertSPtr = std::shared_ptr<Insert>;
}

