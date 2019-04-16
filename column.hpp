#pragma once

#include <memory>
#include <vector>

namespace Sql1
{
	class Errmsg : SysRoot
	{
	public:
		enum struct EType
		{
			INFO,
			WARN,
			FATAL,
		};

		explicit Errmsg(EType type, const std::string& text) : mType{ type }, mText{ text } { }

		void output(std::ostream& os) const override
		{
			switch (mType)
			{
				case EType::INFO:
				{
					os << "I) " << mText;
					break;
				}
				case EType::WARN:
				{
					os << "W) " << mText;
					break;
				}
				case EType::FATAL:
				{
					os << "F) " << mText << " (*)";
					break;
				}
			}

			os << std::endl;
		}

		static Errmsg* info(const std::string& arg)
		{
			return new Errmsg{ EType::INFO, arg };
		}

		static Errmsg* warn(const std::string& arg)
		{
			return new Errmsg{ EType::WARN, arg };
		}

		static Errmsg* fatal(const std::string& arg)
		{
			return new Errmsg{ EType::FATAL, arg };
		}

		bool canIgnore() const
		{
			return mType == EType::FATAL ? false : true;
		}

	private:
		EType mType;
		std::string mText;
	};

	using ErrmsgSPtr = std::shared_ptr<Errmsg>;

	class Coltype : SysRoot
	{
	public:
		enum struct EType
		{
			BIGINT,
			TINYINT,
			SMALLINT,
			MEDIUMINT,
			INT,
			DOUBLE,
			FLOAT,
			DECIMAL,
			VARCHAR,
			CHAR,
			MEDIUMTEXT,
			TEXT,
			LONGTEXT,
			DATETIME,
			TIMESTAMP,
			DATE,
		};

		explicit Coltype(EType type, int widthM=-1, int widthD=-1)
			: mType{ type }, mWidthM{ widthM }, mWidthD{ widthD } { }

		EType getType() const
		{
			return mType;
		}

		int getWidthM() const
		{
			return mWidthM;
		}

		int getWidthD() const
		{
			return mWidthD;
		}

		void output(std::ostream& os) const override
		{
			switch (mType)
			{
				case EType::BIGINT:
				{
					os << "BIGINT";

					break;
				}
				case EType::TINYINT:
				{
					os << "TINYINT";

					break;
				}
				case EType::SMALLINT:
				{
					os << "SMALLINT";

					break;
				}
				case EType::MEDIUMINT:
				{
					os << "MEDIUMINT";

					break;
				}
				case EType::INT:
				{
					os << "INT";

					break;
				}
				case EType::DOUBLE:
				{
					os << "DOUBLE";

					break;
				}
				case EType::FLOAT:
				{
					os << "FLOAT";

					break;
				}
				case EType::DECIMAL:
				{
					os << "DECIMAL";

					break;
				}
				case EType::VARCHAR:
				{
					os << "VARCHAR";

					break;
				}
				case EType::CHAR:
				{
					os << "CHAR";

					break;
				}
				case EType::MEDIUMTEXT:
				{
					os << "MEDIUMTEXT";

					break;
				}
				case EType::TEXT:
				{
					os << "TEXT";

					break;
				}
				case EType::LONGTEXT:
				{
					os << "LONGTEXT";

					break;
				}
				case EType::DATETIME:
				{
					os << "DATETIME";

					break;
				}
				case EType::TIMESTAMP:
				{
					os << "TIMESTAMP";

					break;
				}
				case EType::DATE:
				{
					os << "DATE";

					break;
				}
			}

			if (mWidthM > 0)
			{
				os << "(" << mWidthM;

				if (mWidthD >= 0)
				{
					os << "," << mWidthD;
				}

				os << ")";
			}
		}

		std::string convert() const;

	private:
		EType mType;
		int mWidthM;		// 全体桁数
		int mWidthD;		// 小数点以下の桁数
	};

	using ColtypeSPtr = std::shared_ptr<Coltype>;

	class Defval : SysRoot
	{
	public:
		enum struct EType
		{
			NUL,
			CURRENT_TIMESTAMP,
			STRING,
			INT,
			DOUBLE,
		};

		explicit Defval(EType type, const std::string& value=std::string())
			: mType{ type }, mValue{ value } { }

		void output(std::ostream& os) const override
		{
			switch (mType)
			{
				case EType::NUL:
				{
					os << "NULL";
					break;
				}
				case EType::CURRENT_TIMESTAMP:
				{
					os << "CURRENT_TIMESTAMP";
					break;
				}
				case EType::STRING:
				{
					os << "STRING(" << getValue() << ")";
					break;
				}
				case EType::INT:
				{
					os << "INT(" << getValue() << ")";
					break;
				}
				case EType::DOUBLE:
				{
					os << "DOUBLE(" << getValue() << ")";
					break;
				}
			}
		}

		EType getType() const
		{
			return mType;
		}

		const std::string getValue() const
		{
			switch (mType)
			{
				case EType::NUL:
				{
					return "NULL";
				}
				case EType::CURRENT_TIMESTAMP:
				{
					return "CURRENT_TIMESTAMP";
				}
 				case EType::STRING:
				{
					return std::string("'") + mValue + "'";
				}
				default:
					break;
			}

			return mValue;
		}

	private:
		EType mType;
		std::string mValue;
	};

	using DefvalSPtr = std::shared_ptr<Defval>;

	class Colopt : SysRoot
	{
	public:
		enum struct EType
		{
			UNSIGNED,
			CHARACTER_SET,
			NUL,
			NOT_NUL,
			DEFAULT,
			ON_UPDATE_CURRENTTIMESTAMP,
			AUTO_INCREMENT,
			COMMENT,
		};

		explicit Colopt(EType type, Defval* defval=nullptr) : mType{ type }, mDefval{ defval } { }
		explicit Colopt(EType type, const std::string& text) : mType{ type }, mText{ text } { }

		EType getType() const
		{
			return mType;
		}

		const DefvalSPtr& getDefval() const
		{
			return mDefval;
		}

		void output(std::ostream& os) const override
		{
			switch (mType)
			{
				case EType::UNSIGNED:
				{
					os << "UNSIGNED";

					break;
				}
				case EType::CHARACTER_SET:
				{
					os << "CHARACTER SET(" << mText << ")";

					break;
				}
				case EType::NUL:
				{
					os << "NULL";

					break;
				}
				case EType::NOT_NUL:
				{
					os << "NOT NULL";

					break;
				}
				case EType::DEFAULT:
				{
					os << "DEFAULT(";
					mDefval->output(os);
					os << ")";

					break;
				}
				case EType::ON_UPDATE_CURRENTTIMESTAMP:
				{
					os << "ON UPDATE CURRENTTIMESTAMP";

					break;
				}
				case EType::AUTO_INCREMENT:
				{
					os << "AUTO_INCREMENT";

					break;
				}
				case EType::COMMENT:
				{
					os << "COMMENT(" << mText << ")";

					break;
				}
			}

			os << "/";
		}

		std::string convert() const;

	private:
		EType mType;
		DefvalSPtr mDefval;
		std::string mText;
	};

	using ColoptSPtr = std::shared_ptr<Colopt>;

	class Coldef : SysRoot
	{
	public:
		explicit Coldef(const std::string& name, Coltype* coltype, std::vector<ColoptSPtr>&& colopts)
			: mName{ name }, mColtype{ coltype }, mColopts{ std::move(colopts) } { }

		const std::string& getName() const
		{
			return mName;
		}

		const ColtypeSPtr& getColtype() const
		{
			return mColtype;
		}

		const std::vector<ColoptSPtr>& getColopts() const
		{
			return mColopts;
		}

		void output(std::ostream& os) const override
		{
			os << "name=[" << mName << "] type=[";
			mColtype->output(os);
			os << "]";

			if (! mColopts.empty())
			{
				os << " options=[/" << mColopts << "]";
			}

 			os << std::endl;
		}

		std::string convert() const;

	private:
		std::string mName;
		ColtypeSPtr mColtype;
		std::vector<ColoptSPtr> mColopts;
	};

	using ColdefSPtr = std::shared_ptr<Coldef>;

	// ------------------------------------------------------------------------

	class Refoption : SysRoot
	{
	public:
		enum class EType
		{
			UPDATE,
			DELETE,
		};

		enum class EAction
		{
			RESTRICT,
			CASCADE,
			SET_NUL,
			NO_ACTION,
		};

		explicit Refoption(EType type, EAction action) : mType{ type }, mAction{ action } { }

		void output(std::ostream& os) const override;

		EType getType() const
		{
			return mType;
		}

		EAction getAction() const
		{
			return mAction;
		}

		std::string convert() const;

	private:
		EType mType;
		EAction mAction;
	};

	using RefoptionSPtr = std::shared_ptr<Refoption>;

	inline std::ostream& operator<<(std::ostream& os, Refoption::EType type)
	{
		using EType = Refoption::EType;

		switch (type)
		{
			case EType::UPDATE:
			{
				os << "UPDATE";

				break;
			}
			case EType::DELETE:
			{
				os << "DELETE";

				break;
			}
		}

		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, Refoption::EAction action)
	{
		using EAction = Refoption::EAction;

		switch (action)
		{
			case EAction::RESTRICT:
			{
				os << "RESTRICT";

				break;
			}
			case EAction::CASCADE:
			{
				os << "CASCADE";

				break;
			}
			case EAction::SET_NUL:
			{
				os << "SET NULL";

				break;
			}
			case EAction::NO_ACTION:
			{
				os << "NO ACTION";

				break;
			}
		}

		return os;
	}

	class Tabcond : SysRoot
	{
	public:
		enum class EType
		{
			PRIMARY_KEY,
			UNIQUE_KEY,
			FOREIGN_KEY,
			KEY,
		};

		explicit Tabcond(EType type, std::vector<std::string>&& colnames,
			const std::string& name=std::string())
			: mType{ type }, mColnames{ std::move(colnames) }, mName{ name } { }

		explicit Tabcond(EType type, std::vector<std::string>&& colnames,
			const std::string& name, const std::string& reftabname, std::vector<std::string>&& refcolnames,
			Refoption* refoption=nullptr)
			: mType{ type }, mColnames{ std::move(colnames) }, mName{ name },
			  mReftabname{ reftabname }, mRefcolnames{ std::move(refcolnames) },
			  mRefoption{ refoption } { }

		void output(std::ostream& os) const override
		{
			os << "type=[";

			switch (mType)
			{
				case EType::PRIMARY_KEY:
				{
					os << "primary key";

					break;
				}
				case EType::UNIQUE_KEY:
				{
					os << "unique key";

					break;
				}
				case EType::FOREIGN_KEY:
				{
					os << "foreign key";

					break;
				}
				case EType::KEY:
				{
					os << "key";

					break;
				}
			}

			os << "]";

			if (! mName.empty())
			{
				os << " name=[" << mName << "]";
			}

			std::cout << " columns=[" << mColnames << "]";

			if (! mReftabname.empty())
			{
				os << " refer-to=[" << mReftabname << "(" << mRefcolnames << ")]";
			}

			if (mRefoption)
			{
				os << " ";
				mRefoption->output(os);
			}

			os << std::endl;
		}

		EType getType() const
		{
			return mType;
		}

		const std::vector<std::string>& getColnames() const
		{
			return mColnames;
		}

		const std::string& getName() const
		{
			return mName;
		}

		const std::string& getReftabname() const
		{
			return mReftabname;
		}

		const std::vector<std::string>& getRefcolnames() const
		{
			return mRefcolnames;
		}

		const RefoptionSPtr& getRefoption() const
		{
			return mRefoption;
		}

		std::string convert1() const;
		std::string convert2(const std::string& tabname) const;

	private:
		EType mType;
		std::vector<std::string> mColnames;
		std::string mName;
		std::string mReftabname;
		std::vector<std::string> mRefcolnames;
		RefoptionSPtr mRefoption;
	};

	using TabcondSPtr = std::shared_ptr<Tabcond>;

	class Table : SysRoot
	{
	public:
		explicit Table(const std::string& name,
			std::vector<ColdefSPtr>&& coldefs, std::vector<TabcondSPtr>&& tabconds)
			: mName{ name }, mColdefs{ std::move(coldefs) }, mTabconds{ std::move(tabconds) } { }

		void output(std::ostream& os) const override
		{
			os << "TABLE=" << mName << std::endl << std::endl;

			os << indent_manip::push;

				os << "* COLUMN" << std::endl;
				os << indent_manip::push;
				os << mColdefs << std::endl;
				os << indent_manip::pop;

				os << "* CONDITION" << std::endl;
				os << indent_manip::push;
				os << mTabconds << std::endl;
				os << indent_manip::pop;

				const auto errors = checkSpannerSyntax();

				if (! errors.empty())
				{
					os << "* CONVERT MESSAGE" << std::endl;
					os << indent_manip::push;
					os << errors << std::endl;
					os << indent_manip::pop;
				}

			os << indent_manip::pop;
		}

		std::vector<ErrmsgSPtr> checkSpannerSyntax() const;
		std::string convert() const;

	private:
		std::string mName;
		std::vector<ColdefSPtr> mColdefs;
		std::vector<TabcondSPtr> mTabconds;
	};
}

