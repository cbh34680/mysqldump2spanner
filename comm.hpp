#pragma once

#if defined(__clang__)
	#define CPP_CPLUS			(1)
#elif defined(__GNUG__)
	#define CPP_GNUG			(1)
#elif defined(_MSC_VER)
	#define CPP_MSVC			(1)
#else
	#error "unknown compiler"
#endif

#if defined(__GNUC__)
	#define UNUSED				__attribute__((unused))
#else
	#define UNUSED
#endif

#if defined(CPP_MSVC)
	#pragma warning(default:4716)
#endif

// https://github.com/spacemoose/ostream_indenter
#include "indent_facet.hpp"

#define Rvalue(a)		std::move(a)
#define Forward(a)		std::forward<decltype(a)>(a)

#define IsTrue			!!
#define Not				!

#define EndL			'\n'

namespace Sql1
{
	using Strings = std::vector<std::string>;

	struct MainConfig
	{
		bool print_version = false;
		bool print_help = false;
		unsigned long int insert_limit = 1000UL;
		bool no_drop_table = false;
	};

	class BadTypeError : public std::runtime_error { public: using runtime_error::runtime_error; };

	class SysRoot
	{
	public:
		virtual ~SysRoot() = default;

		virtual void output(std::ostream& os) const = 0;
	};

	template<typename T, typename std::enable_if< std::is_base_of<SysRoot, T>::value >::type* = nullptr>
	inline std::ostream& operator<<(std::ostream& os, const std::vector<std::shared_ptr<T>>& arg)
	{
		for (const auto& e: arg)
		{
			e->output(os);
		}

		return os;
	}

	// ex.) const vector<string>& arg
	//
	inline std::string join_strs(const Strings& strs, const char* delim = "")
	{
		if (strs.empty())
		{
			return "";
		}

		std::ostringstream ss;
		std::copy(std::begin(strs), std::end(strs), std::ostream_iterator<std::string>(ss, delim));

		auto s = ss.str();
		s.erase(s.length() - std::char_traits<char>::length(delim));

		return s;
	}

	inline std::ostream& operator<<(std::ostream& os, const Strings& arg)
	{
/*
		for (const auto& e: arg)
		{
			os << e << ",";
		}
*/
		os << join_strs(arg, ",");

		return os;
	}

	inline bool is_allowed_name(const std::string& arg)
	{
		const char* p = arg.c_str();

		while (*p)
		{
			if ('a' <= *p && *p <= 'z')
			{
			}
			else if ('A' <= *p && *p <= 'Z')
			{
			}
			else if ('0' <= *p && *p <= '9')
			{
			}
			else if (*p == '_')
			{
			}
			else
			{
				return false;
			}

			++p;
		}

		return true;
	}

	class Errmsg : public SysRoot
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


	class Stmt : public SysRoot
	{
	public:
		virtual std::string convert() const { return std::move(std::string()); }
		virtual std::vector<ErrmsgSPtr> checkSpannerSyntax() const { return std::vector<ErrmsgSPtr>(); }
	};

	using StmtSPtr = std::shared_ptr<Stmt>;
}

