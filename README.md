# arg, a C++ command-line parser

This C++ library faciliates the processing of command-line arguments through a `arg::Parser` object. After creation, the parser is informed of options that it will parse for and their disposition. The `argc` and `argv` from the signature of `main` function are then passed to the parser to activate the magic.

Chun-Chung Chen <cjj@u.washington.edu>
