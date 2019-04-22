#include "std.hpp"
#include "comm.hpp"
#include "ddl.hpp"

void Sql1::Refoption::output(std::ostream& os) const
{
	os << "on=[" << mType << "(" << mAction << ")]";
}

std::vector<Sql1::ErrmsgSPtr> Sql1::DropTable::checkSpannerSyntax() const
{
	std::vector<ErrmsgSPtr> errors;

	if (! is_allowed_name(mName))
	{
		errors.emplace_back(Errmsg::fatal(S_("table=[") + mName + "] " + T_("Name violates the rules")));
	}

	if (mIfExists)
	{
		errors.emplace_back(Errmsg::warn(S_("'IF EXISTS' ") + T_("is ignored")));
	}

	return std::move(errors);
}

std::vector<Sql1::ErrmsgSPtr> Sql1::CreateTable::checkSpannerSyntax() const
{
	std::vector<ErrmsgSPtr> errors;

	if (! is_allowed_name(mName))
	{
		errors.emplace_back(Errmsg::fatal(S_("table=[") + mName + "] " + T_("Name violates the rules")));
	}

	for (const auto& coldef: mColdefs)
	{
		const std::string key{ std::string{ "column=[" } + coldef->getName() + "]: " };

		if (! is_allowed_name(coldef->getName()))
		{
			errors.emplace_back(Errmsg::fatal(key + T_("Name violates the rules")));
		}

		const auto& coltype = coldef->getColtype();

		if (coltype->getType() == Coltype::EType::TINYINT)
		{
			if (coltype->getWidthM() == 1)
			{
				errors.emplace_back(Errmsg::info(key + T_("Converted to BOOL")));
			}
		}

		for (const auto& colopt: coldef->getColopts())
		{
			std::string ignore_str;

			switch (colopt->getType())
			{
				case Colopt::EType::AUTO_INCREMENT:
				{
					errors.emplace_back(Errmsg::fatal(key + "AUTO_INCREMENT " + T_("can not be used")));
					break;
				}
				case Colopt::EType::UNSIGNED:
				{
					ignore_str = "UNSIGNED";
					break;
				}
				case Colopt::EType::CHARACTER_SET:
				{
					ignore_str = "'CHARACTER SET'";
					break;
				}
				case Colopt::EType::DEFAULT:
				{
					const auto& defval = colopt->getDefval();

					if (defval->getType() != Defval::EType::NUL)
					{
						ignore_str = S_("DEFAULT(") + defval->getValue() + ")";
					}

					break;
				}
				case Colopt::EType::COMMENT:
				{
					ignore_str = S_("COMMENT('") + colopt->getText() + "')";

					break;
				}
				default:
					break;
			}

			if (! ignore_str.empty())
			{
				errors.emplace_back(Errmsg::warn(key + ignore_str + ' ' + T_("is ignored")));
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
							errors.emplace_back(Errmsg::fatal(key + "ON UPDATE " + T_("can not be used")));

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
							errors.emplace_back(Errmsg::warn(key + T_("Converted to 'NO ACTION'")));

							break;
						}
						case Refoption::EAction::SET_NUL:
						{
							errors.emplace_back(Errmsg::fatal(key + "'SET NULL' " + T_("is ignored")));

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

		errors.emplace_back(Errmsg::fatal(T_("Primary key is required")));
	}

	if (num_fk > 1)
	{
		errors.emplace_back(Errmsg::fatal(T_("Only one can be specified as a foreign key")));
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

std::string Sql1::Tabcond::convert1(bool noInterleaveInParent) const
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
			if (noInterleaveInParent)
			{
				break;
			}

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

std::string Sql1::Tabcond::convert2(const std::string& tabname, bool noCreateIndex) const
{
	std::string ret;

	switch (mType)
	{
		case Tabcond::EType::UNIQUE_KEY:
		{
			ret = S_("CREATE UNIQUE INDEX ") + tabname + '_' + mName + " ON ";
			ret += tabname + " (" + join_strs(mColnames, ",") + ")";

			break;
		}
		case Tabcond::EType::KEY:
		{
			if (noCreateIndex)
			{
				break;
			}

			ret = S_("CREATE INDEX ") + tabname + '_' + mName + " ON ";
			ret += tabname + " (" + join_strs(mColnames, ",") + ")";

			break;
		}
		default:
			break;
	}

	return std::move(ret);
}

std::string Sql1::DropTable::convert() const
{
	std::string ret{ "DROP TABLE " };

	ret += mName + " \n;\n";

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

	Strings strs;

	for (const auto& tabcond: mTabconds)
	{
		const std::string s{ tabcond->convert1(mNoCreateIndex) };

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
		const std::string s{ tabcond->convert2(mName, mNoInterleaveInParent) };

		if (! s.empty())
		{
			ret += s + "\n;\n";
		}
	}

	return std::move(ret);
}

