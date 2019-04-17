#include "std.hpp"
#include "scanner.hpp"

#include <cstring>

const int indent_manip::index = std::ios_base::xalloc();

/*
	http://www.jonathanbeard.io/tutorials/FlexBisonC++
	https://github.com/jonathan-beard/simple_wc_example

	http://www.asahi-net.or.jp/~wg5k-ickw/html/online/flex-2.5.4/flex_7.html
	http://d.hatena.ne.jp/coiledcoil/20130324/1364136044
*/

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

