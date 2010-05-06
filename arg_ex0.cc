#include <arg.hh>
#include <iostream>

int main(int argc, char ** argv)
{
	arg::Parser p;
	int n;
	p.add_opt('n').stow(n);
	p.parse(argc, argv);
	std::cout << n << '\n';
	return 0;
}
