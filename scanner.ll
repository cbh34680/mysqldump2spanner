/*
//
// [original]
// https://gist.github.com/codebrainz/2933703 (c99.l)
//
*/

%option c++
%option yyclass="Sql1::Scanner"

%option noyywrap noinput nounput batch stack caseless

/*
%option outfile="scanner.cc"
%option outfile="scanner.cc" header-file="scanner.hh"

%option yylineno
%option debug
*/

%{
	#include "std.hpp"

	#if defined(WIN32) || defined(_MSC_VER)
		#define YY_NO_UNISTD_H (1)
	#endif

	#undef YY_DECL
	#define YY_DECL	Sql1::Parser::symbol_type Sql1::Scanner::Scan_(Driver& driver_)

	#include "scanner.hpp"

	#define yyterminate()		return Parser::make_END(drv_loc)

	#define YY_USER_ACTION		drv_loc.columns(YYLeng());

	//#define YY_USER_ACTION		user_action(drv_loc);
	//#define YY_USER_INIT		user_init(drv_loc);

	#define drv_ctx				driver_.context
	#define drv_loc				drv_ctx->locat

	#define S1_FLEX_WARN(a)		driver_.error(std::cerr, a, __FILE__, __LINE__)

	namespace Sql1
	{
		static Parser::symbol_type makeSymbol_IS_DEC(const char* arg_cstr, location& arg_loc);
		static Parser::symbol_type makeSymbol_FS_DEC(const char* arg_cstr, location& arg_loc);
	}
%}

D			[0-9]
L			[a-zA-Z_]

H			[a-fA-F0-9]
E			([Ee][+-]?{D}+)
P			([Pp][+-]?{D}+)
FS			(f|F|l|L)
IS			((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

ESC			\\
SQ			\'
DQ			\"

%x COMMSTATE STRSTATE QIDSTATE

%%

%{
	/* MEMO: scanner.ll rule section */

	drv_loc.step();
%}

<COMMSTATE>{
	\n				{
						drv_loc.lines();
						drv_loc.step();
					}
	"*/"			{
						drv_loc.step();
						yy_pop_state();
					}
	.				{
						drv_loc.step();
					}
}

<STRSTATE>{
	\n				{
						drv_loc.lines();
						drv_loc.step();

						S1_FLEX_WARN("Unterminated string");

						yy_pop_state();
					}
	\t				{
						drv_loc.step();

						temp_str += "\\t";
					}
	\\.				{
						drv_loc.step();

						temp_str += YYText();
					}
	[^']			{
						drv_loc.step();

						temp_str += YYText();
					}
	'				{
						drv_loc.step();
						yy_pop_state();

						return Parser::make_STRING_LITERAL(temp_str, drv_loc);
					}
}

<QIDSTATE>{
	\n				{
						drv_loc.lines();
						drv_loc.step();

						S1_FLEX_WARN("Unterminated string");

						yy_pop_state();
					}
	[^`]			{
						drv_loc.step();

						temp_str += YYText();
					}
	`				{
						drv_loc.step();
						yy_pop_state();

						return Parser::make_IDENTIFIER(temp_str, drv_loc);
					}
}

<INITIAL>{
	"/*"			{
						drv_loc.step();
						yy_push_state(COMMSTATE);
					}
	'				{
						temp_str = "";

						drv_loc.step();
						yy_push_state(STRSTATE);
					}
	`				{
						temp_str = "";

						drv_loc.step();
						yy_push_state(QIDSTATE);
					}

	"//"[^\n]*				{ drv_loc.step(); }
	"--"[^\n]*				{ drv_loc.step(); }

	"="						{ return Parser::make_ASSIGN(drv_loc); }
	","						{ return Parser::make_COMMA(drv_loc); }
	";"						{ return Parser::make_SEMICOL(drv_loc); }
	"("						{ return Parser::make_OPENPAR(drv_loc); }
	")"						{ return Parser::make_CLOSEPAR(drv_loc); }

	"CREATE"				{ return Parser::make_CREATE(drv_loc); }
	"DROP"					{ return Parser::make_DROP(drv_loc); }
	"LOCK"					{ return Parser::make_LOCK(drv_loc); }
	"UNLOCK"				{ return Parser::make_UNLOCK(drv_loc); }
	"TABLE"					{ return Parser::make_TABLE(drv_loc); }
	"TABLES"				{ return Parser::make_TABLES(drv_loc); }
	"IF"					{ return Parser::make_IF(drv_loc); }
	"EXISTS"				{ return Parser::make_EXISTS(drv_loc); }
	"WRITE"					{ return Parser::make_WRITE(drv_loc); }

	"INSERT"				{ return Parser::make_INSERT(drv_loc); }
	"INTO"					{ return Parser::make_INTO(drv_loc); }
	"VALUES"				{ return Parser::make_VALUES(drv_loc); }
	"UPDATE"				{ return Parser::make_UPDATE(drv_loc); }
	"DELETE"				{ return Parser::make_DELETE(drv_loc); }

	"BIGINT"				{ return Parser::make_BIGINT(drv_loc); }
	"TINYINT"				{ return Parser::make_TINYINT(drv_loc); }
	"SMALLINT"				{ return Parser::make_SMALLINT(drv_loc); }
	"MEDIUMINT"				{ return Parser::make_MEDIUMINT(drv_loc); }
	"INT"					{ return Parser::make_INT(drv_loc); }
	"DOUBLE"				{ return Parser::make_DOUBLE(drv_loc); }
	"FLOAT"					{ return Parser::make_FLOAT(drv_loc); }
	"DECIMAL"				{ return Parser::make_DECIMAL(drv_loc); }
	"VARCHAR"				{ return Parser::make_VARCHAR(drv_loc); }
	"CHAR"					{ return Parser::make_CHAR(drv_loc); }
	"MEDIUMTEXT"			{ return Parser::make_MEDIUMTEXT(drv_loc); }
	"TEXT"					{ return Parser::make_TEXT(drv_loc); }
	"LONGTEXT"				{ return Parser::make_LONGTEXT(drv_loc); }
	"DATETIME"				{ return Parser::make_DATETIME(drv_loc); }
	"TIMESTAMP"				{ return Parser::make_TIMESTAMP(drv_loc); }
	"DATE"					{ return Parser::make_DATE(drv_loc); }

	"UNSIGNED"				{ return Parser::make_UNSIGNED(drv_loc); }
	"CHARACTER"				{ return Parser::make_CHARACTER(drv_loc); }
	"SET"					{ return Parser::make_SET(drv_loc); }
	"NOT"					{ return Parser::make_NOT(drv_loc); }
	"NULL"					{ return Parser::make_NUL(drv_loc); }
	"DEFAULT"				{ return Parser::make_DEFAULT(drv_loc); }
	"CURRENT_TIMESTAMP"		{ return Parser::make_CURRENT_TIMESTAMP(drv_loc); }
	"ON"					{ return Parser::make_ON(drv_loc); }
	"COMMENT"				{ return Parser::make_COMMENT(drv_loc); }

	"UNIQUE"				{ return Parser::make_UNIQUE(drv_loc); }
	"PRIMARY"				{ return Parser::make_PRIMARY(drv_loc); }
	"CONSTRAINT"			{ return Parser::make_CONSTRAINT(drv_loc); }
	"FOREIGN"				{ return Parser::make_FOREIGN(drv_loc); }
	"KEY"					{ return Parser::make_KEY(drv_loc); }
	"REFERENCES"			{ return Parser::make_REFERENCES(drv_loc); }
	"CASCADE"				{ return Parser::make_CASCADE(drv_loc); }
	"NO"					{ return Parser::make_NO(drv_loc); }
	"ACTION"				{ return Parser::make_ACTION(drv_loc); }
	"RESTRICT"				{ return Parser::make_RESTRICT(drv_loc); }

	"ENGINE"				{ return Parser::make_ENGINE(drv_loc); }
	"AUTO_INCREMENT"		{ return Parser::make_AUTOINCR(drv_loc); }
	"CHARSET"				{ return Parser::make_CHARSET(drv_loc); }

	({L}|$)({L}|{D}|$)*		{ return Parser::make_IDENTIFIER(YYText(), drv_loc); }

	0{IS}?					{ return makeSymbol_IS_DEC(YYText(), drv_loc); }
	[1-9]{D}*{IS}?			{ return makeSymbol_IS_DEC(YYText(), drv_loc); }

	{D}*"."{D}+{E}?{FS}?	{ return makeSymbol_FS_DEC(YYText(), drv_loc); }
	{D}+"."{D}*{E}?{FS}?	{ return makeSymbol_FS_DEC(YYText(), drv_loc); }

	[ \t]+					{
								drv_loc.step();
							}
	\r?\n					{
								drv_loc.lines();
								drv_loc.step();
							}
	.						{
								S1_FLEX_WARN(S_("invalid character [") + YYText() + "]");
							}
}

%%

/* MEMO: scanner.ll c++ code section */

#if defined(YY_DECL_IS_OURS)

	#error "YY_DECL no defined !!"
#endif

Sql1::Parser::symbol_type Sql1::Scanner::Scan(Driver& driver_)
{
	return Scan_(driver_);
}

#include <cctype>
#include <cstring>

static std::string to_lower_(const char* arg_cstr)
{
	std::string org{ arg_cstr };

	std::string dst;
	dst.resize(org.size());

	std::transform(std::begin(org), std::end(org), std::begin(dst), ::tolower);

	return dst;
}

// https://stackoverflow.com/questions/8715213/why-is-there-no-stdstou
//
static unsigned int stou_(std::string const & str, size_t * idx = 0, int base = 10)
{
	unsigned long result = std::stoul(str, idx, base);

	if (result > std::numeric_limits<unsigned>::max())
	{
		throw std::out_of_range("stou");
	}

	return result;
}

static Sql1::Parser::symbol_type Sql1::makeSymbol_IS_DEC(const char* arg_cstr, location& arg_loc)
{
	const std::string str{ to_lower_(arg_cstr) };
	const bool un_signed{ str.find("u") != std::string::npos };

	if (str.find("ll") != std::string::npos)
	{
		if (un_signed)
		{
			return Parser::make_ULLONG_LITERAL(std::stoull(str), arg_loc);
		}
		else
		{
			return Parser::make_LLONG_LITERAL(std::stoll(str), arg_loc);
		}
	}

	if (str.find("l") != std::string::npos)
	{
		if (un_signed)
		{
			return Parser::make_ULONG_LITERAL(std::stoul(str), arg_loc);
		}
		else
		{
			return Parser::make_LONG_LITERAL(std::stol(str), arg_loc);
		}
	}

	if (un_signed)
	{
		return Parser::make_UINT_LITERAL(stou_(str), arg_loc);
	}

	return Parser::make_INT_LITERAL(std::stoi(str), arg_loc);
}

static Sql1::Parser::symbol_type Sql1::makeSymbol_FS_DEC(const char* arg_cstr, location& arg_loc)
{
	const std::string str{ to_lower_(arg_cstr) };

	if (str.find("l") != std::string::npos)
	{
		return Parser::make_LDOUBLE_LITERAL(std::stold(str), arg_loc);
	}

	if (str.find("f") != std::string::npos)
	{
		return Parser::make_FLOAT_LITERAL(std::stof(str), arg_loc);
	}

	return Parser::make_DOUBLE_LITERAL(std::stod(str), arg_loc);
}

