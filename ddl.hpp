#pragma once

namespace Sql1
{
	class Coltype : public SysRoot
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

		int getWidth() const
		{
			return getWidthM();
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

	class Defval : public SysRoot
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
					return S_("'") + mValue + "'";
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

	class Colopt : public SysRoot
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

	class Coldef : public SysRoot
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

	class Refoption : public SysRoot
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

	class Tabcond : public SysRoot
	{
	public:
		enum class EType
		{
			PRIMARY_KEY,
			UNIQUE_KEY,
			FOREIGN_KEY,
			KEY,
		};

		explicit Tabcond(EType type, Strings&& colnames,
			const std::string& name=std::string())
			: mType{ type }, mColnames{ std::move(colnames) }, mName{ name } { }

		explicit Tabcond(EType type, Strings&& colnames,
			const std::string& name, const std::string& reftabname, Strings&& refcolnames,
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

			os << " columns=[" << mColnames << "]";

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

		const Strings& getColnames() const
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

		const Strings& getRefcolnames() const
		{
			return mRefcolnames;
		}

		const RefoptionSPtr& getRefoption() const
		{
			return mRefoption;
		}

		std::string convert1(bool noInterleaveInParent) const;
		std::string convert2(const std::string& tabname, bool noCreateIndex) const;

	private:
		EType mType;
		Strings mColnames;
		std::string mName;
		std::string mReftabname;
		Strings mRefcolnames;
		RefoptionSPtr mRefoption;
	};

	using TabcondSPtr = std::shared_ptr<Tabcond>;

	class IgnoreDdl : public Stmt
	{
	public:
		explicit IgnoreDdl(const std::string& text) : mText{ text } { }

		void output(std::ostream& os) const override
		{
			os << "ddl-type=[ignore] text=[" << mText << "]" << std::endl << std::endl;
		}

	private:
		std::string mText;
	};

	class DropTable : public Stmt
	{
	public:
		explicit DropTable(const std::string& name, bool if_exists)
			: mName{ name }, mIfExists{ if_exists } { }

		void output(std::ostream& os) const override
		{
			os << "ddl-type=[drop] table=[" << mName << "]" << std::endl << std::endl;

			os << indent_manip::push;

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

		std::string convert() const override;
		std::vector<ErrmsgSPtr> checkSpannerSyntax() const override;

	private:
		std::string mName;
		bool mIfExists;
	};

	class CreateTable : public Stmt
	{
	public:
		explicit CreateTable(const std::string& name,
			std::vector<ColdefSPtr>&& coldefs, std::vector<TabcondSPtr>&& tabconds)
			: mName{ name }, mColdefs{ std::move(coldefs) }, mTabconds{ std::move(tabconds) } { }

		void setNoInterleaveInParent(bool arg)
		{
			mNoInterleaveInParent = arg;
		}

		void setNoCreateIndex(bool arg)
		{
			mNoCreateIndex = arg;
		}

		void output(std::ostream& os) const override
		{
			os << "ddl-type=[create] table=[" << mName << "]" << std::endl << std::endl;

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

		Strings getColnames() const
		{
			Strings ret;

			for (const auto& coldef: mColdefs)
			{
				ret.push_back(coldef->getName());
			}

			return std::move(ret);
		}

		const ColdefSPtr& getColdef(int n) const
		{
			assert(n < static_cast<decltype(n)>(mColdefs.size()));

			return mColdefs.at(n);
		}

		std::string convert() const override;
		std::vector<ErrmsgSPtr> checkSpannerSyntax() const override;

	private:
		std::string mName;
		std::vector<ColdefSPtr> mColdefs;
		std::vector<TabcondSPtr> mTabconds;

		bool mNoInterleaveInParent = false;
		bool mNoCreateIndex = false;
	};

	using CreateTableSPtr = std::shared_ptr<CreateTable>;
}

