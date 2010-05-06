#include <arg.hh>
using namespace arg;
int main(int argc, char **argv)
{
	Parser p;
	SubParser * sp = new SubParser;
	int va1 = 0;
	sp->add_opt("param1").stow(va1)
		.help("parameter 1")
		.show_default();
	double va2 = 1.0;
	sp->add_opt("param2").stow(va2)
		.help("parameter 2")
		.show_default();
	int op;
	sp->add_opt("opt1").set(& op, 1)
		.help("perform op 1");
	sp->add_opt("opt2").set(& op, 2)
		.help("perform op 2");
	sp->add_opt_help();
	p.add_opt('o', "options").store(sp);
	p.add_opt_help();
	p.parse(argc, argv);
	return 0;
}
