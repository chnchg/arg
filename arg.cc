/* arg.cc
 *
 * Copyright (C) 2010,2017 Chun-Chung Chen <cjj@u.washington.edu>
 * 
 * This file is part of arg.
 * 
 * arg is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with arg.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "arg.hh"
#include <iostream>
#include <algorithm>
#include <cstdlib>

using namespace arg;
using namespace std;

Value::~Value() {}

void Value::set(std::string const &) {}

string Value::to_str() const
{
	return string();
}

string Value::get_type() const
{
	return "unknown";
}

Option::Option(int key, const string & name) :
	key(key),
	name(name),
	store_optional(false),
	set_bool(nullptr),
	bool_value(false),
	set_var(nullptr),
	set_once(false),
	call_func(nullptr),
	help_default(false)
{}

Option::~Option() {}

Option & Option::store(std::shared_ptr<Value> ptr)
{
	if (! ptr) ptr = std::make_shared<Value>(); // null storage
	store_ptr = ptr;
	return * this;
}

Option & Option::optional(const string & str)
{
	store_optional = true;
	store_str = str;
	return * this;
}

Option & Option::set(bool & var, bool value)
{
	set_bool = & var;
	bool_value = value;
	return * this;
}

Option & Option::set(int * var, int value)
{
	set_var = var;
	set_value = value;
	return * this;
}

Option & Option::once(int init)
{
	set_once = true;
	set_init = init;
	return * this;
}

Option & Option::call(CallBack * func, void * data)
{
	call_func = func;
	call_data = data;
	return * this;
}

Option & Option::help(const string & text, const string & var)
{
	help_text = text;
	if (var.size()) help_var = var;
	return * this;
}

Option & Option::help_word(const string & var)
{
	help_var = var;
	return * this;
}

Option & Option::show_default(bool do_show)
{
	help_default = do_show;
	return * this;
}

bool Option::take_value()
{
	return bool(store_ptr);
}

bool Option::need_value()
{
	return store_ptr && ! store_optional;
}

int Option::get_key()
{
	return key;
}

const string & Option::get_name()
{
	return name;
}

string Option::get_help(HelpFormat format)
{
	string h;
	bool s = isprint(key) && ! isspace(key);

	switch (format) {
	case HF_REGULAR:
		h = s ? (string("  -") + char(key)) : "    ";
		if (name != "") h += (s ? ", --" : "  --") + name;
		if (store_ptr) {
			if (store_optional) h += "[=" + help_var + "]";
			else h += (name == "" ? " " : "=") + help_var;
		}
		if (h.size() < 26) h.resize(26, ' ');
		h += "   ";
		h += help_text;
		break;
	case HF_NODASH:
		h = "    ";
		if (s) h += char(key);
		if (name != "") h += (s ? ", " : "") + name;
		if (store_ptr) {
			if (name == "") {
				if (store_optional) h += " [" + help_var + "]";
				else h += " " + help_var;
			}
			else {
				if (store_optional) h += "[=" + help_var + "]";
				else h += "=" + help_var;
			}
		}
		if (h.size() < 26) h.resize(26, ' ');
		h += "   ";
		h += help_text;
		break;
	}
	if (help_default && store_ptr) { // append " (default: ...)" to help
		h += " (default: ";
		h += store_ptr->to_str();
		h += ")";
	}
	return h;
}

void Option::process()
{
	if (store_ptr) {
		if (! store_optional) throw OptError(name, "missing value");
		store_ptr->set(store_str);
	}
	if (set_bool) {
		* set_bool = bool_value;
	}
	if (set_var) {
		if (set_once && set_init != * set_var) throw OptError(name, "can not re-set");
		* set_var = set_value;
	}
	if (call_func) {
		if (! (* call_func)(key, "", call_data)) throw OptError(name, "callback error");
	}
}

void Option::process(const string & str)
{
	if (! store_ptr) throw OptError(name, "unwanted value '" + str + "'");
	store_ptr->set(str);
	if (set_bool) {
		* set_bool = bool_value;
	}
	if (set_var) {
		if (set_once && set_init != * set_var) throw OptError(name, "can not re-set");
		* set_var = set_value;
	}
	if (call_func) {
		if (! (* call_func)(key, str, call_data)) throw OptError(name, "callback error");
	}
}

Argument::Argument(const string & name) :
	name(name)
{}

Argument::~Argument() {}

Argument & Argument::store(std::shared_ptr<Value> ptr)
{
	store_ptr = ptr ? ptr : std::make_shared<Value>();
	return * this;
}

Argument & Argument::help(const string & text)
{
	help_text = text;
	return * this;
}

const string & Argument::get_name()
{
	return name;
}

std::string Argument::get_help()
{
	string h = "    " + name;
	if (h.size() < 26) h.resize(26, ' ');
	h += "   ";
	h += help_text;
	return h;
}

void Argument::process(std::string const & str)
{
	if (! store_ptr) throw OptError(name, "no place to store '" + str + "'");
	store_ptr->set(str);
}

Parser::HelpLine::HelpLine(std::string const & m, std::shared_ptr<Option> o) :
	msg(m),
	opt(o)
{}

Parser::~Parser() {}

void Parser::add_help(string const & msg)
{
	help_list.emplace_back(msg, nullptr);
}

Option & Parser::add_opt(int key, string const & name, bool hide)
{
	auto o = std::make_shared<Option>(key, name);
	opt_list.push_back(o);
	if (! hide) help_list.emplace_back("", o);
	return * o;
}

Option & Parser::add_opt(const string & name, bool hide)
{
	return add_opt(0, name, hide);
}

vector<string> & Parser::args()
{
	return arg_strs;
}

void Parser::parse(int argc, char * argv[], bool ignore_unknown)
{
	arg_strs.clear();
	prog_name = argv[0];
	for (int i = 1; i < argc; i ++) { // skip program name
		string s = argv[i];
		if (s[0] != '-') { // non-option => argument
			arg_strs.push_back(s);
			continue;
		}
		if (s[1] == '-') { // long options
			string n; // name
			string v; // value
			bool vp = false; // value exists?
			string::size_type k = s.find('=');
			if (k != string::npos) { // value present
				n = s.substr(2, k - 2);
				v = s.substr(k + 1);
				vp = true;
			} else {
				n = s.substr(2);
			}
			// find option from list
			auto j = std::find_if(opt_list.begin(), opt_list.end(), [&n](std::shared_ptr<Option> x){return x->get_name() == n;});
			if (j != opt_list.end()) {
				if (vp) (*j)->process(v);
				else (*j)->process(v);
			}
			else if (! ignore_unknown) throw UnknError(n);
			continue;
		}
		// short options
		for (string::size_type k = 1; k < s.length(); k ++) { // there can be several options in a token
			auto j = std::find_if(opt_list.begin(), opt_list.end(), [&s,k](shared_ptr<Option> x){return x->get_key() == s[k];});
			if (j == opt_list.end()) {
				if (! ignore_unknown) throw UnknError(string("-") + s[k]);
				break; // for unknown option ignore the rest of the token
			}
			else {
				if (! (* j)->take_value()) { // no value allowed
					(* j)->process();
					continue;
				}
				// value allowed, it could follow
				string v = s.substr(k + 1);
				if (v != "") {
					(* j)->process(v);
					break;
				}
				if (! (* j)->need_value() || ++ i >= argc) {
					(* j)->process();
					break;
				} else (* j)->process(argv[i]);
				break;
			}
		}
	}
	if (arg_list.size()) {
		if (arg_list.size() != arg_strs.size()) throw Error("number of arguments mismatch");
		for (size_t i = 0; i < arg_list.size(); i ++) arg_list[i]->process(arg_strs[i]);
	}
}

void Parser::set_header(std::string const & text)
{
	header_text = text;
}

const string & Parser::get_header() const
{
	return header_text;
}

std::shared_ptr<Option> Parser::find(int key)
{
	for (auto j: opt_list) if (j->get_key() == key) return j;
	return nullptr;
}

std::shared_ptr<Option> Parser::find(std::string const & name)
{
	for (auto j: opt_list) if (j->get_name() == name) return j;
	return nullptr;
}

void Parser::remove(int key)
{
	// erase help first (have to leave the extra help lines.)
	help_list.erase(std::remove_if(help_list.begin(), help_list.end(), [key](HelpLine & h){
		return h.opt && h.opt->get_key() == key;
	}), help_list.end());
	// erase the option itself
	opt_list.erase(std::remove_if(opt_list.begin(), opt_list.end(), [key](std::shared_ptr<Option> x){
		return x->get_key() == key;
	}), opt_list.end());
}

void Parser::remove(std::string const & name)
{
	// erase help first (have to leave the extra help lines.)
	help_list.erase(std::remove_if(help_list.begin(), help_list.end(), [&](HelpLine & l){
		return l.opt && l.opt->get_name() == name;
	}), help_list.end());
	// erase the option itself
	opt_list.erase(std::remove_if(opt_list.begin(), opt_list.end(), [&](std::shared_ptr<Option> x){
		return x->get_name() == name;
	}), opt_list.end());
}

void Parser::remove_all()
{
	help_list.clear();
	opt_list.clear();
}

string Parser::get_help()
{
	string h;
	if (arg_list.size()) {
		h += "Usage: ";
		h += prog_name + " [Options]";
		for (auto i = arg_list.begin(); i != arg_list.end(); i ++) {
			h += " " + (* i)->get_name();
		}
		h += "\n\n";
	}
	if (help_list.size()) h += " Valid options are:\n\n";
	for (auto i = help_list.begin(); i != help_list.end(); i ++) {
		h += i->msg;
		if (i->opt) {
			h += i->opt->get_help();
		}
		h += '\n';
	}
	if (arg_list.size()) {
		h += "\n Required argument";
		if (arg_list.size() > 1) h += 's';
		h += ":\n";
		for (auto i = arg_list.begin(); i != arg_list.end(); i ++) {
			h += '\n' + (* i)->get_help();
		}
		h += "\n";
	}
	return h;
}

namespace { // local callback functions
	bool help_callback(int, const string &, void * data)
	{
		Parser * p = static_cast<Parser *>(data);
		cout << p->get_header();
		cout << '\n';
		cout << p->get_help();
		cout << '\n';
		exit(0);
	}

	bool version_callback(int, const string &, void * data)
	{
		string * s = static_cast<string *>(data);
		cout << * s << '\n';
		exit(0);
	}
}

Option & Parser::add_opt_help()
{
	return add_opt('h', "help")
		.call(& help_callback, this)
		.help("display this help list and exit");
}

Option & Parser::add_opt_version(const string & ver)
{
	version_info = ver;
	return add_opt('V', "version")
		.call(& version_callback, & version_info)
		.help("print program version and exit");
}

Argument & Parser::add_arg(string const & name)
{
	arg_list.emplace_back(std::make_shared<Argument>(name));
	return * arg_list.back();
}

SubParser::SubParser() :
	sep(',')
{
}

void SubParser::set(const string & str)
{
	string name;
	string value;
	int s = 0;
	string::size_type n = string::npos;
	for (string::size_type k = 0; k < str.length(); k ++) {
		if (str[k] == '=') n = k;
		if (k + 1 == str.length() || str[k + 1] == sep) {
			if (n == string::npos) n = k + 1;
			name = str.substr(s, n - s);

			auto j = std::find_if(opt_list.begin(), opt_list.end(), [name](std::shared_ptr<Option> x){
				return x->get_name() == name;
			});
			if (j == opt_list.end()) throw UnknError(name);

			if (n < k + 1) {
				value = str.substr(n + 1, k - n);
				(* j)->process(value);
			} else (* j)->process();
			k ++;
			s = k + 1;
			n = string::npos;
		}
	}
}

string SubParser::get_help()
{
	string h;
	for (auto & i: help_list) {
		h += i.msg;
		if (i.opt) h += i.opt->get_help(Option::HF_NODASH);
		h += '\n';
	}
	return h;
}

namespace { // local callback functions
	bool sub_help_callback(int, const string &, void * data)
	{
		SubParser * p = static_cast<SubParser *>(data);
		cout << '\n';
		cout << p->get_help();
		cout << '\n';
		exit(0);
	}
}

Option & SubParser::add_opt_help()
{
	return add_opt("help")
		.call(& sub_help_callback, this)
		.help("display this help list and exit");
}

Error::Error() {}

Error::Error(string const & m)
{
	msg = m;
}

string Error::get_msg()
{
	return msg;
}

OptError::OptError(string const & o)
{
	opt = o;
	msg = "error processing option: " + opt;
}

OptError::OptError(string const & o, string const & m)
{
	opt = o;
	msg = m + " for option: " + o;
}

ConvError::ConvError(string const & str, string const & type)
{
	msg = "error converting '" + str + "' to " + type;
}

UnknError::UnknError(string const & o)
{
	msg = "unknown option: " + o;
}

MissingError::MissingError(string const & type)
{
	msg = "missing an argument of type " + type;
}
