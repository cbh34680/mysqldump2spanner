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

	return parser.parse();
}

#include <unistd.h>

bool parse_args(int argc, char** argv, Sql1::MainConfig& config)
{
	bool ret{ true };

	int optc;

	while ((optc = getopt(argc, argv, "vhDi:")) != -1)
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
			case 'i':
			{
				char* e = nullptr;

				unsigned long int ul = strtoul(optarg, &e, 0);

				if (*e)
				{
					std::cerr << "ERROR: " << e << std::endl;

					ret = false;
				}

				if (ul > 10000UL)		// hard limit
				{
					config.insert_limit = 10000UL;
				}
				else
				{
					config.insert_limit = ul;
				}

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

		if (! ret)
		{
			break;
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

		std::cerr << "\t" << "-h" << "\t" << T_("display this help and exit") << std::endl;

		std::cerr << "\t" << "-v" << "\t" << T_("output version information and exit") << std::endl;

		std::cerr << "\t" << "-i num" << "\t" << T_("limit the number of VALUE at insert")
													<< " (default 1000, max 10000)" << std::endl;

		std::cerr << "\t" << "-D" << "\t" << T_("Do not generate 'DROP TABLE'") << std::endl;

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

