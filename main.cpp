#include "std.hpp"
#include "scanner.hpp"
#include "comm.hpp"

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

static int parse_file(const char* arg_filename, const Sql1::MainConfig& config)
{
	Sql1::Context context{ arg_filename, &config };
	Sql1::Driver driver{ &context };

	Sql1::Scanner scanner;
	Sql1::Parser parser{ scanner, driver };

	//context.debug_pos = true;
	parser.set_debug_level(0);

	const int rc = parser.parse();

/*
	std::cout << std::endl;
	std::cout << "-- " << __FILE__ << "(" << __LINE__ << ") " << __func__ << " -->" << std::endl;
	std::cout << context;
	std::cout << "-- " << __FILE__ << "(" << __LINE__ << ") " << __func__ << " --<" << std::endl;
*/

	return rc;
}

#include <unistd.h>

bool parse_args(int argc, char** argv, Sql1::MainConfig& config)
{
	bool ret{ true };

	int optc;

	while ((optc = getopt(argc, argv, "vhD")) != -1)
	{
		switch (optc)
		{
			case 'v':
			{
				config.print_version = true;

				break;
			}
			case 'h':
			{
				config.print_help = true;

				break;
			}
			case 'D':
			{
				config.no_drop_table = true;

				break;
			}
			default:
			{
				std::cerr << "'" << static_cast<char>(optc) << "': unknown option" << std::endl;

				ret = false;

				break;
			}
		}
	}

	return ret;
}

int main(int argc, char** argv)
{
	Sql1::MainConfig config;

	if (! parse_args(argc, argv, config))
	{
		std::cerr << "- Argument Parse error." << std::endl;

		return 1;
	}

	if (config.print_version)
	{
		std::cerr << "mysqldump2spanner 1.0.0" << std::endl;

		return 0;
	}

	if (config.print_help)
	{
		std::cerr << "Usage: " << argv[0] << " [OPTIONS] < [FILE]" << std::endl << std::endl;

		std::cerr << "\t" << "-h" << "\t" << "この使い方を表示して終了する" << std::endl;
		std::cerr << "\t" << "-v" << "\t" << "バージョン情報を表示して終了する" << std::endl;
		std::cerr << "\t" << "-D" << "\t" << "DROP TABLE を生成しません" << std::endl;

		return 0;
	}

	//TraceOnOff t1{ &std::cout };
	//TraceOnOff t2{ &std::cerr };

	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	const auto parse_rc = parse_file("<stdin>", config);

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

