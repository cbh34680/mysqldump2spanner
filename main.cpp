#include "std.hpp"
#include "impl.hpp"

#include <regex>

const int indent_manip::index = std::ios_base::xalloc();

/*
	http://www.jonathanbeard.io/tutorials/FlexBisonC++
	https://github.com/jonathan-beard/simple_wc_example

	http://www.asahi-net.or.jp/~wg5k-ickw/html/online/flex-2.5.4/flex_7.html
	http://d.hatena.ne.jp/coiledcoil/20130324/1364136044
*/

void Sql1::Refoption::output(std::ostream& os) const
{
	os << "on=[" << mType << "(" << mAction << ")]";
}

std::vector<Sql1::ErrmsgSPtr> Sql1::CreateTable::checkSpannerSyntax() const
{
	std::vector<ErrmsgSPtr> errors;

	if (! is_allowed_name(mName))
	{
		errors.emplace_back(Errmsg::fatal(std::string("table=[") + mName + "] 規則違反の名前です"));
	}

	for (const auto& coldef: mColdefs)
	{
		const std::string key{ std::string{ "column=[" } + coldef->getName() + "]: " };

		if (! is_allowed_name(coldef->getName()))
		{
			errors.emplace_back(Errmsg::fatal(key + "規則違反の名前です"));
		}

		const auto& coltype = coldef->getColtype();

		if (coltype->getType() == Coltype::EType::TINYINT)
		{
			if (coltype->getWidthM() == 1)
			{
				errors.emplace_back(Errmsg::info(key + "BOOL に変換されます"));
			}
		}

		for (const auto& colopt: coldef->getColopts())
		{
			switch (colopt->getType())
			{
				case Colopt::EType::UNSIGNED:
				{
					errors.emplace_back(Errmsg::warn(key + "UNSIGNED は無視されます"));
					break;
				}
				case Colopt::EType::CHARACTER_SET:
				{
					errors.emplace_back(Errmsg::warn(key + "CHARACTER SET は無視されます"));
					break;
				}
				case Colopt::EType::DEFAULT:
				{
					const auto& defval = colopt->getDefval();

					std::string s{ key + "DEFAULT(" };
					s += defval->getValue() + ")";

					switch (defval->getType())
					{
						case Defval::EType::NUL:
						{
							break;
						}
						case Defval::EType::CURRENT_TIMESTAMP:
						{
							errors.emplace_back(Errmsg::warn(s + " は無視されます"));

							break;
						}
						default:
						{
							errors.emplace_back(Errmsg::fatal(s + " は使用できません"));

							break;
						}
					}

					break;
				}
				case Colopt::EType::AUTO_INCREMENT:
				{
					errors.emplace_back(Errmsg::fatal(key + "AUTO_INCREMENT は使用できません"));
					break;
				}
				case Colopt::EType::COMMENT:
				{
					errors.emplace_back(Errmsg::warn(key + "COMMENT は無視されます"));
					break;
				}
				default:
					break;
			}
		}
	}

	int num_pk{ 0 }, num_fk{ 0 };

	for (const auto& tabcond: mTabconds)
	{
		switch (tabcond->getType())
		{
			case Tabcond::EType::PRIMARY_KEY:
			{
				++num_pk;

				break;
			}
			case Tabcond::EType::FOREIGN_KEY:
			{
				const std::string key{ std::string{ "key=[" } + tabcond->getName() + "]: " };

				if (const auto& refopt = tabcond->getRefoption())
				{
					switch (refopt->getType())
					{
						case Refoption::EType::UPDATE:
						{
							errors.emplace_back(Errmsg::fatal(key + "ON UPDATE は使用できません"));

							break;
						}
						case Refoption::EType::DELETE:
						{

							break;
						}
					}

					switch (refopt->getAction())
					{
						case Refoption::EAction::RESTRICT:
						{
							errors.emplace_back(Errmsg::warn(key + "NO ACTION に変更されます"));

							break;
						}
						case Refoption::EAction::SET_NUL:
						{
							errors.emplace_back(Errmsg::fatal(key + "SET NULL は使用できません"));

							break;
						}
						default:
							break;
					}
				}

				++num_fk;

				break;
			}
			default:
				break;
		}
	}

	if (num_pk != 1)
	{
		assert(num_pk == 0);

		errors.emplace_back(Errmsg::fatal("主キーは必須です"));
	}

	if (num_fk > 1)
	{
		errors.emplace_back(Errmsg::fatal("外部キーに指定できるのは 1 つのみです"));
	}

	return std::move(errors);
}

std::string Sql1::Coltype::convert() const
{
	std::string ret;

	switch (mType)
	{
		case EType::SMALLINT:
		case EType::MEDIUMINT:
		case EType::INT:
		case EType::BIGINT:
		{
			ret = "INT64";

			break;
		}
		case EType::TINYINT:
		{
			if (mWidthM == 1)
			{
				ret = "BOOL";
			}
			else
			{
				ret = "INT64";
			}

			break;
		}
		case EType::FLOAT:
		case EType::DOUBLE:
		case EType::DECIMAL:
		{
			ret = "FLOAT64";

			break;
		}
		case EType::CHAR:
		case EType::VARCHAR:
		{
			ret = "STRING(";
			ret += std::to_string(mWidthM);
			ret += ")";

			break;
		}
		case EType::TEXT:
		{
			ret = "STRING(65535)";

			break;
		}
		case EType::MEDIUMTEXT:
		case EType::LONGTEXT:
		{
			ret = "STRING(MAX)";

			break;
		}
		case EType::DATETIME:
		case EType::TIMESTAMP:
		{
			ret = "TIMESTAMP";

			break;
		}
		case EType::DATE:
		{
			ret = "DATE";

			break;
		}
	}

	return std::move(ret);
}

std::string Sql1::Colopt::convert() const
{
	std::string ret;

	switch (mType)
	{
		case EType::NUL:
		{
			ret = "NULL";

			break;
		}
		case EType::NOT_NUL:
		{
			ret = "NOT NULL";

			break;
		}
		case EType::ON_UPDATE_CURRENTTIMESTAMP:
		{
			ret = "OPTIONS (allow_commit_timestamp=true)";

			break;
		}
		default:
			break;
	}

	return std::move(ret);
}

std::string Sql1::Coldef::convert() const
{
	std::string ret{ mName + " " };

	ret += mColtype->convert();

	for (const auto& colopt: mColopts)
	{
		const std::string s{ colopt->convert() };

		if (! s.empty())
		{
			ret += " ";
			ret += s;
		}
	}

	return std::move(ret);
}

std::string Sql1::Refoption::convert() const
{
	std::stringstream ss;

	ss << mType<< " " << mAction;

	return std::move(ss.str());
}

std::string Sql1::Tabcond::convert1() const
{
	std::string ret;

	switch (mType)
	{
		case Tabcond::EType::PRIMARY_KEY:
		{
			ret = std::string{ "PRIMARY KEY(" } + join_strs(mColnames, ",") + ")";

			break;
		}
		case Tabcond::EType::FOREIGN_KEY:
		{
			ret = std::string{ "INTERLEAVE IN PARENT " } + mReftabname;

			if (mRefoption)
			{
				ret += " ON ";
				ret += mRefoption->convert();
			}

			break;
		}
		default:
			break;
	}

	return std::move(ret);
}

std::string Sql1::Tabcond::convert2(const std::string& tabname) const
{
	std::string ret;

	bool unique{ false };

	switch (mType)
	{
		case Tabcond::EType::UNIQUE_KEY:
		{
			unique = true;
		}
		case Tabcond::EType::KEY:
		{
			ret += "CREATE ";
			ret += unique ? "UNIQUE " : "";
			ret += "INDEX ";
			ret += mName + " ON ";
			ret += tabname + " (";
			ret += join_strs(mColnames, ",") + ")";

			break;
		}
		default:
			break;
	}

	return std::move(ret);
}

std::string Sql1::CreateTable::convert() const
{
	std::string ret{ "CREATE TABLE " };

	ret += mName + " (\n";

	for (const auto& coldef: mColdefs)
	{
		ret += "  ";

		ret += coldef->convert() + ",";
		ret += "\n";
	}

	ret += ")";

	// --- PRIMARY KEY, INTERLEAVE IN PARENT

	std::vector<std::string> strs;

	for (const auto& tabcond: mTabconds)
	{
		const std::string s{ tabcond->convert1() };

		if (! s.empty())
		{
			strs.push_back(s);
		}
	}

	if (! strs.empty())
	{
		ret += " ";
		ret += join_strs(strs, ",\n  ");
	}

	ret += "\n;\n";

	// --- CREATE INDEX

	for (const auto& tabcond: mTabconds)
	{
		const std::string s{ tabcond->convert2(mName) };

		if (! s.empty())
		{
			ret += s + "\n;\n";
		}
	}

	return std::move(ret);
}

struct TraceOnOff
{
	class NullStreambuf : public std::streambuf { };

	NullStreambuf nullst;
	std::ostream* os;
	std::streambuf* backup;

	TraceOnOff(std::ostream* arg_os) : os{ arg_os }
	{
		backup = os->rdbuf(&nullst);
	}

	~TraceOnOff()
	{
		os->rdbuf(backup);
	}
};

static int parse(const char* arg_filename, int dbglv)
{
	Sql1::Context context{ arg_filename };
	Sql1::Driver driver{ &context };

	Sql1::Scanner scanner;
	Sql1::Parser parser{ scanner, driver };

	//context.debug_pos = true;
	parser.set_debug_level(dbglv);

	const int rc = parser.parse();

/*
	std::cout << std::endl;
	std::cout << "-- " << __FILE__ << "(" << __LINE__ << ") " << __func__ << " -->" << std::endl;
	std::cout << context;
	std::cout << "-- " << __FILE__ << "(" << __LINE__ << ") " << __func__ << " --<" << std::endl;
*/

	return rc;
}

int main(int argc, char** argv)
{
	//TraceOnOff t1{ &std::cout };
	//TraceOnOff t2{ &std::cerr };

	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	int dbglv = 0;

	for (int i=0; i<argc; ++i)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			dbglv = 1;
		}
	}

	const auto parse_rc = parse("<stdin>", dbglv);

	if (parse_rc)
	{
		std::cerr << "- Parse error." << std::endl;

		return 1;
	}

	std::cerr << "- Parse success." << std::endl;

	std::ios_base::sync_with_stdio(true);

	return 0;
}

// https://sourceforge.net/projects/winflexbison/

