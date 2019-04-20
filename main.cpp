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
				std::cerr << "unknown option" << std::endl;

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

static void print_help(std::ostream& os, const char* pgname)
{
	os << "Usage: " << pgname << " [OPTIONS] < [FILE]" << std::endl << std::endl;

	os << "\t" << "-h" << "\t" << T_("display this help and exit") << std::endl;

	os << "\t" << "-v" << "\t" << T_("output version information and exit") << std::endl;

	os << "\t" << "-i num" << "\t" << T_("limit the number of values at insert")
													<< " (default 1000, max 10000)" << std::endl;

	os << "\t" << "-D" << "\t" << T_("Do not generate 'DROP TABLE'") << std::endl;
}

#define DEFINE_TO_STRING_(a)		#a
#define DEFINE_TO_STRING(a)			DEFINE_TO_STRING_(a)

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

#if defined(TEXTDOMAINDIR)
	bindtextdomain("mysqldump2spanner", DEFINE_TO_STRING(TEXTDOMAINDIR));
#else
	if (const char* tddir = getenv("TEXTDOMAINDIR"))
	{
		bindtextdomain("mysqldump2spanner", tddir);
	}
	else
	{
		bindtextdomain("mysqldump2spanner", ".");
	}
#endif

	textdomain("mysqldump2spanner");

	Sql1::MainConfig config;

	if (! parse_args(argc, argv, config))
	{
		std::cerr << std::endl;

		print_help(std::cerr, argv[0]);

		return 1;
	}

	if (config.print_help)
	{
		print_help(std::cerr, argv[0]);

		return 0;
	}

	if (config.print_version)
	{
		std::cerr << "mysqldump2spanner 1.0.0" << std::endl;

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

