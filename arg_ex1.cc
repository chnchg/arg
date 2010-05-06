#include <arg.hh>
#include <iostream>
using namespace std;
const string version = "1.0";

int main(int argc, char ** argv)
{
	// system parameters
	// setup parser
	arg::Parser parser;
	parser.set_header("arg Testing Program v" + version);
	parser.add_help("");
	parser.add_help("available options are:");
	int n;
	parser.add_opt('n', "number").stow(n = 10)
		.help("set number of nodes to INT", "INT")
		.show_default();
	string f;
	int f_given = 0;
	parser.add_opt('i', "input").stow(f)
		.help("read data from FILE", "FILE")
		.set(& f_given).once();
	parser.add_opt_help();
	parser.add_opt_version(version);
	// parse command line
	try {
		parser.parse(argc, argv);
	}
	catch (arg::Error e) {
		cout << "Error parsing command line: " << e.get_msg() << '\n';
		return 1;
	}
	// check for parameter consistency
	if (! f.size()) {
		cout << "Need to specify the input file!\n";
		return 1;
	}
	// output
	cout << "The parameters are:\n"
		  << "number = " << n << '\n'
		  << "input = " << f << '\n';
	return 0;
}
