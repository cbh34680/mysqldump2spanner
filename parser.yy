/* MEMO 0: parser.yy define section */

%code top
{
	// MEMO : code top (cc)

	#include "std.hpp"
}

%language "c++"
%skeleton "lalr1.cc"
%require "3.0.4"

%defines "parser.hh"
/*
%output "parser.cc"
*/

%define parser_class_name {Parser}

%define api.namespace {Sql1}
%define api.token.constructor
%define api.token.prefix {TOK_}
%define api.value.type variant

%define parse.assert
%define parse.trace
%define parse.error verbose

%locations

%parse-param	{ Sql1::Scanner& scanner_ }
%param			{ Sql1::Driver& driver_ }
%printer		{ yyoutput << $$; } <*>;

/*
%expect 1
*/

%code requires
{
	// MEMO : code requires (hh)

	#include "comm.hpp"
	#include "ddl.hpp"
	#include "dml.hpp"

	namespace Sql1
	{
		class Scanner;
		class Driver;
	}
}

%code provides
{
	// MEMO : code provides (hh)
}

%{
	// MEMO : user first part
%}

%code
{
	// MEMO : code (cc)

	#include "scanner.hpp"

	#undef yylex
	#define yylex				scanner_.Scan

	#define drv_ctx				driver_.context

	#define S1_WARN(a)			driver_.error(std::cerr, a, __FILE__, __LINE__)
	#define S1_ERROR(a)			driver_.error(std::cerr, a, __FILE__, __LINE__); YYABORT
	#define S1_ASSERT(a, b)		if (! (a)) { S1_ERROR((b)); }
	#define S1_SWAP(a, b)		if (! (b)) { S1_ERROR("swap source is null"); } std::swap((a), (b))

	static std::map<std::string, Sql1::Strings> colnames_map;
	static std::shared_ptr<Sql1::Insert> curr_insert;
	static int curr_values = 0;
}

%initial-action
{
	// MEMO : initial-action
	
	@$.begin.filename = @$.end.filename = &drv_ctx->filename;

}

// https://gist.github.com/codebrainz/2933703 (c99.y) -->

%token
	END		0			"End Of File"

	ASSIGN				"="
	COMMA				","
	SEMICOL				";"
	OPENPAR				"("
	CLOSEPAR			")"

	CREATE				"create"
	DROP				"drop"
	LOCK				"lock"
	UNLOCK				"unlock"
	TABLE				"table"
	TABLES				"tables"
	IF					"if"
	EXISTS				"exists"
	WRITE				"write"

	INSERT				"insert"
	INTO				"into"
	VALUES				"values"
	UPDATE				"update"
	DELETE				"delete"

	BIGINT				"bigint"
	TINYINT				"tinyint"
	SMALLINT			"smallint"
	MEDIUMINT			"mediumint"
	INT					"int"
	DOUBLE				"double"
	FLOAT				"float"
	DECIMAL				"decimal"
	VARCHAR				"varchar"
	CHAR				"char"
	MEDIUMTEXT			"mediumtext"
	TEXT				"text"
	LONGTEXT			"longtext"
	DATETIME			"datetime"
	TIMESTAMP			"timestamp"
	DATE				"date"

	UNSIGNED			"unsigned"
	CHARACTER			"character"
	SET					"set"
	NOT					"not"
	NUL					"null"
	DEFAULT				"default"
	CURRENT_TIMESTAMP	"current_timestamp"
	ON					"on"
	COMMENT				"comment"

	UNIQUE				"unique"
	PRIMARY				"primary"
	CONSTRAINT			"constraint"
	FOREIGN				"foreign"
	KEY					"key"
	REFERENCES			"references"
	CASCADE				"cascade"
	NO					"no"
	ACTION				"action"
	RESTRICT			"restrict"

	ENGINE				"engine"
	AUTOINCR			"auto_increment"
	CHARSET				"charset"

	;

// ---------------------------------------------------------------------------

%token
	<std::string>					IDENTIFIER
	<std::string>					STRING_LITERAL
	<int>							INT_LITERAL
	<unsigned int>					UINT_LITERAL
	<long int>						LONG_LITERAL
	<unsigned long int>				ULONG_LITERAL
	<long long int>					LLONG_LITERAL
	<unsigned long long int>		ULLONG_LITERAL
	<float>							FLOAT_LITERAL
	<double>						DOUBLE_LITERAL
	<long double>					LDOUBLE_LITERAL
	;

// ---------------------------------------------------------------------------

%type
	<bool>							if_exists
	;

%type
	<int>							width
	;

%type
	<std::string>					ident
									value
	;

%type
	<Coldef*>						coldef
	;

%type
	<Coltype*>						coltype
	;

%type
	<Colopt*>						colopt
	;

%type
	<Defval*>						defval
	;

%type
	<Tabcond*>						tabcond
	;

%type
	<StmtSPtr>						ddl
	;

%type
	<Refoption*>					refopt.or.empty
	;

%type
	<Refoption::EType>				refopt.type
	;

%type
	<Refoption::EAction>			refopt.action
	;

%type
	<std::vector<ColdefSPtr>>		coldef.list
	;

%type
	<std::vector<ColoptSPtr>>		colopt.list.or.empty
									colopt.list
	;

%type
	<std::vector<TabcondSPtr>>		tabcond.list.or.empty
									tabcond.list

%type
	<Strings>						ident.list
									colnames.or.empty
									value.list
	;

// ---------------------------------------------------------------------------

%start start

// ---------------------------------------------------------------------------

%%

start
	: %empty
		{
		}
	| stmt.list[stmts]
		{
		}
	;

stmt.list
	: stmt.list[orig] stmt
		{
		}
	| stmt
		{
		}
	;

stmt
	: ";"
		{
		}
	| ddl ";"
		{
			if ($ddl)
			{
				$ddl->output(std::cerr);

				const auto errors = $ddl->checkSpannerSyntax();

				if (! errors.empty())
				{
					for (const auto& e: errors)
					{
						if (! e->canIgnore())
						{
							S1_ERROR("続行できない障害を検出しました");
						}
					}
				}

				const std::string spn_ddl = $ddl->convert();

				if (! spn_ddl.empty())
				{
					std::cout << spn_ddl;
				}
			}
		}
	| dml ";"
		{
		}
	;

ddl
	: "create" "table" ident[tabname] "(" coldef.list[coldefs]
		tabcond.list.or.empty[tabconds] ")" tabopt.list.or.empty
		{
			assert(colnames_map.find($tabname) == colnames_map.end());

			auto* o = new CreateTable{ $tabname, std::move($coldefs), std::move($tabconds) };

			colnames_map[$tabname] = std::move(o->getColnames());

			$$.reset(o);
		}
	| "drop" "table" if_exists ident[tabname]
		{
			$$.reset(new DropTable{ $tabname, $if_exists });
		}
	| "lock" "tables" ident[tabname] "write"
		{
			$$.reset(new IgnoreDdl{ std::string("LOCK TABLE ") + $tabname });
		}
	| "unlock" "tables"
		{
			$$.reset(new IgnoreDdl{ std::string("UNLOCK TABLES") });
		}
	;

dml
	: "insert" "into" ident[tabname] colnames.or.empty[colnames] "values"
		{
			Strings colnames{ std::move($colnames) };

			if (colnames.empty())
			{
				if (colnames_map.find($tabname) == colnames_map.end())
				{
					S1_ERROR(std::string("[") + $tabname + "]: 認識できないテーブル名です");
				}

				colnames = colnames_map.at($tabname);
			}

			curr_insert = std::unique_ptr<Insert>(new Insert{ $tabname, std::move(colnames) });
			curr_values = 0;

			curr_insert->output(std::cerr);

			std::cerr << std::endl;
		}
	  values.list
		{
			std::cerr << indent_manip::push;
			std::cerr << "! " << curr_values << " record writed." << std::endl << std::endl;
			std::cerr << indent_manip::pop;

			std::cout << "\n;\n";
		}
	;

colnames.or.empty
	: %empty
		{
		}
	| "(" ident.list[orig] ")"
		{
			$$ = std::move($orig);
		}
	;

values.list
	: values.list "," packed.value.list
		{
		}
	| packed.value.list
		{
		}
	;

packed.value.list
	: "(" value.list[values] ")"
		{
			if ((curr_values % 1000) == 0)
			{
				if (curr_values)
				{
					std::cout << "\n;\n";
				}

				// "insert into (...) values "
				//
				std::cout << curr_insert->convert() << " VALUES ";
			}
			else
			{
				std::cout << ",";
			}

			std::cout << "(" << join_strs($values, ",") << ")";

			++curr_values;
		}
	;

value.list
	: value.list[orig] "," value
		{
			$$ = std::move($orig);

			$$.push_back(std::move($value));
		}
	| value
		{
			$$.push_back(std::move($value));
		} 
	;

value
	: NUL
		{
			$$ = "NULL";
		}
	| STRING_LITERAL[orig]
		{
			$$ = std::move(std::string("'") + $orig + "'");
		}
	| INT_LITERAL[orig]
		{
			$$ = std::move(std::to_string($orig));
		}
	| DOUBLE_LITERAL[orig]
		{
			$$ = std::move(std::to_string($orig));
		}
	;

if_exists
	: %empty
		{
			$$ = false;
		}
	| "if" "exists"
		{
			$$ = true;
		}
	;

/* --------------------------------------- */

coldef.list
	: coldef.list[orig] "," coldef
		{
			$$ = std::move($orig);

			$$.emplace_back($coldef);
		}
	| coldef
		{
			$$.emplace_back($coldef);
		}
	;

coldef
	: ident coltype colopt.list.or.empty[colopts]
		{
			$$ = new Coldef{ $ident, $coltype, std::move($colopts) };
		}
	;

coltype
	: "bigint" width
		{
			$$ = new Coltype{ Coltype::EType::BIGINT, $width };
		}
	| "tinyint" width
		{
			$$ = new Coltype{ Coltype::EType::TINYINT, $width };
		}
	| "smallint" width
		{
			$$ = new Coltype{ Coltype::EType::SMALLINT, $width };
		}
	| "mediumint" width
		{
			$$ = new Coltype{ Coltype::EType::MEDIUMINT, $width };
		}
	| "int" width
		{
			$$ = new Coltype{ Coltype::EType::INT, $width };
		}
	| "double"
		{
			$$ = new Coltype{ Coltype::EType::DOUBLE };
		}
	| "float"
		{
			$$ = new Coltype{ Coltype::EType::FLOAT };
		}
	| "decimal" "(" INT_LITERAL[m] "," INT_LITERAL[d] ")"
		{
			$$ = new Coltype{ Coltype::EType::DECIMAL, $m, $d };
		}
	| "varchar" width
		{
			$$ = new Coltype{ Coltype::EType::VARCHAR, $width };
		}
	| "char" width
		{
			$$ = new Coltype{ Coltype::EType::CHAR, $width };
		}
	| "mediumtext"
		{
			$$ = new Coltype{ Coltype::EType::MEDIUMTEXT };
		}
	| "text"
		{
			$$ = new Coltype{ Coltype::EType::TEXT };
		}
	| "longtext"
		{
			$$ = new Coltype{ Coltype::EType::LONGTEXT };
		}
	| "datetime"
		{
			$$ = new Coltype{ Coltype::EType::DATETIME };
		}
	| "timestamp"
		{
			$$ = new Coltype{ Coltype::EType::TIMESTAMP };
		}
	| "date"
		{
			$$ = new Coltype{ Coltype::EType::DATE };
		}
	;

width
	: "(" INT_LITERAL[orig] ")"
		{
			$$ = $orig;
		}
	;


colopt.list.or.empty
	: %empty
		{
		}
	| colopt.list[orig]
		{
			$$ = std::move($orig);
		}
	;

colopt.list
	: colopt.list[orig] colopt
		{
			$$ = std::move($orig);

			$$.emplace_back($colopt);
		}
	| colopt
		{
			$$.emplace_back($colopt);
		}
	;

colopt
	: "unsigned"
		{
			$$ = new Colopt{ Colopt::EType::UNSIGNED };
		}
	| "character" "set" IDENTIFIER[text]
		{
			$$ = new Colopt{ Colopt::EType::CHARACTER_SET, $text };
		}
	| "null"
		{
			$$ = new Colopt{ Colopt::EType::NUL };
		}
	| "not" "null"
		{
			$$ = new Colopt{ Colopt::EType::NOT_NUL };
		}
	| "default" defval
		{
			$$ = new Colopt{ Colopt::EType::DEFAULT, $defval };
		}
	| "on" "update" "current_timestamp"
		{
			$$ = new Colopt{ Colopt::EType::ON_UPDATE_CURRENTTIMESTAMP };
		}
	| "auto_increment"
		{
			$$ = new Colopt{ Colopt::EType::AUTO_INCREMENT };
		}
	| "comment" STRING_LITERAL[text]
		{
			$$ = new Colopt{ Colopt::EType::COMMENT, $text };
		}
	;

defval
	: "null"
		{
			$$ = new Defval{ Defval::EType::NUL };
		}
	| "current_timestamp"
		{
			$$ = new Defval{ Defval::EType::CURRENT_TIMESTAMP };
		}
	| STRING_LITERAL[orig]
		{
			$$ = new Defval{ Defval::EType::STRING, $orig };
		}
	| INT_LITERAL[orig]
		{
			$$ = new Defval{ Defval::EType::INT, std::to_string($orig) };
		}
	| DOUBLE_LITERAL[orig]
		{
			$$ = new Defval{ Defval::EType::DOUBLE, std::to_string($orig) };
		}
	;

/* --------------------------------------- */

tabcond.list.or.empty
	: %empty
		{
		}
	| "," tabcond.list[orig]
		{
			$$ = std::move($orig);
		}
	;

tabcond.list
	: tabcond.list[orig] "," tabcond
		{
			$$ = std::move($orig);

			$$.emplace_back($tabcond);
		}
	| tabcond
		{
			$$.emplace_back($tabcond);
		}
	;

tabcond
	: "primary" "key" "(" ident.list[names] ")"
		{
			$$ = new Tabcond{ Tabcond::EType::PRIMARY_KEY, std::move($names) };
		}
	| "unique" "key" ident "(" ident.list[names] ")"
		{
			$$ = new Tabcond{ Tabcond::EType::UNIQUE_KEY, std::move($names), $ident };
		}
	| "constraint" ident[keyname] "foreign" "key" "(" ident.list[names] ")"
		"references" ident[reftab] "(" ident.list[refcols] ")" refopt.or.empty[refopt]
		{
			$$ = new Tabcond{
				Tabcond::EType::FOREIGN_KEY, std::move($names), $keyname,
				$reftab, std::move($refcols), $refopt
			};
		}
	| "key" ident "(" ident.list[names] ")"
		{
			$$ = new Tabcond{ Tabcond::EType::KEY, std::move($names), $ident };
		}
	;

refopt.or.empty
	: %empty
		{
			$$ = nullptr;
		}
	| "on" refopt.type[type] refopt.action[action]
		{
			$$ = new Refoption{ $type, $action };
		}
	;

refopt.type
	: "update"
		{
			$$ = Refoption::EType::UPDATE;
		}
	| "delete"
		{
			$$ = Refoption::EType::DELETE;
		}
	;

refopt.action
	: "restrict"
		{
			$$ = Refoption::EAction::RESTRICT;
		}
	| "cascade"
		{
			$$ = Refoption::EAction::CASCADE;
		}
	| "set" "null"
		{
			$$ = Refoption::EAction::SET_NUL;
		}
	| "no" "action"
		{
			$$ = Refoption::EAction::NO_ACTION;
		}
	;

ident.list
	: ident.list[orig] "," ident
		{
			$$ = std::move($orig);

			$$.push_back(std::move($ident));
		}
	| ident
		{
			$$.push_back(std::move($ident));
		}
	;

ident
	: IDENTIFIER[orig]
		{
			$$ = std::move($orig);
		}
	;

/* --------------------------------------- */

tabopt.list.or.empty
	: %empty
		{
		}
	| tabopt.list
		{
		}
	;

tabopt.list
	: tabopt.list[orig] tabopt
		{
		}
	| tabopt
		{
		}
	;

tabopt
	: "engine" "=" IDENTIFIER
		{
		}
	| "auto_increment" "=" INT_LITERAL
		{
		}
	| "default" "charset" "=" IDENTIFIER
		{
		}
	| "comment" "=" STRING_LITERAL
		{
		}
	;

%%

//https://gist.github.com/codebrainz/2933703 (c99.y) <--

/* MEMO : parser.yy c++ code section */

void Sql1::Parser::error(const location& arg_loc, const std::string& msg)
{
	driver_.error(std::cerr, arg_loc, msg, __FILE__, __LINE__);
}

