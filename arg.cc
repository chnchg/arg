/* arg.cc
 *
 * Copyright (C) 2010 Chun-Chung Chen <cjj@u.washington.edu>
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

#include <arg.hh>
#include <iostream>
#include <cstdlib>

using namespace arg;
using namespace std;

Value::~Value()
{
}

void Value::set(const std::string & str)
{
}

string Value::to_str() const
{
	return string();
}

Option::Option(int k, const string & n) :
	store_ptr(0),
	store_optional(false),
	set_var(0),
	set_once(false),
	call_func(0),
	help_default(false)
{
	key = k;
	name = n;
}

Option::~Option()
{
	if (store_ptr) {
		delete store_ptr;
	}
}

Option & Option::store(Value * ptr)
{
	if (! ptr) ptr = new Value; // null storage
	store_ptr = ptr;
	return * this;
}

Option & Option::optional(const string & str)
{
	store_optional = true;
	store_str = str;
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
	return store_ptr;
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
			else h += "=" + help_var;
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
			if (store_optional) h += "[=" + help_var + "]";
			else h += "=" + help_var;
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
	if (set_var) {
		if (set_once && set_init != * set_var) throw OptError(name, "can not re-set");
		* set_var = set_value;
	}
	if (call_func) {
		if (! (* call_func)(key, str, call_data)) throw OptError(name, "callback error");
	}
}

Parser::~Parser()
{
	while (opt_list.size())	{
		delete opt_list.back();
		opt_list.pop_back();
	}
}

void Parser::add_help(const string & msg)
{
	HelpLine hl;
	hl.msg = msg;
	hl.opt = 0;
	help_list.push_back(hl);
}

Option & Parser::add_opt(int key, const string & name, bool hide)
{
	Option * o = new Option(key, name);
	opt_list.push_back(o);
	if (! hide) {
		HelpLine hl;
		hl.msg = "";
		hl.opt = o;
		help_list.push_back(hl);
	}
	return * o;
}

Option & Parser::add_opt(const string & name, bool hide)
{
	return add_opt(0, name, hide);
}

vector<string> & Parser::args()
{
	return arg_list;
}

void Parser::parse(int argc, char * argv[], bool ignore_unknown)
{
	arg_list.clear();
	for (int i = 1; i < argc; i ++) { // skip program name
		string s = argv[i];
		if (s[0] != '-') { // non-option => argument
			arg_list.push_back(s);
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
			bool unknown = true;
			for (vector<Option *>::iterator j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_name() == n) {
				unknown = false;
				if (vp) (* j)->process(v);
				else (* j)->process();
				break;
			}
			if (unknown && ! ignore_unknown) throw UnknError(n);
			continue;
		}
		// short options
		for (string::size_type k = 1; k < s.length(); k ++) { // there can be several options in a token
			vector<Option *>::iterator j;
			for (j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_key() == s[k]) break;
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
}

void Parser::set_header(const std::string & text)
{
	header_text = text;
}

const string & Parser::get_header() const
{
	return header_text;
}

Option * Parser::find(int key)
{
	for (vector<Option *>::iterator j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_key() == key) {
		return * j;
	}
	return 0;
}

Option * Parser::find(const std::string & name)
{
	for (vector<Option *>::iterator j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_name() == name) {
		return * j;
	}
	return 0;
}

void Parser::remove(int key)
{
	// erase help first (have to leave the extra help lines.)
	for (vector<HelpLine>::iterator i = help_list.begin(); i != help_list.end(); i ++) if (i->opt && i->opt->get_key() == key) {
		help_list.erase(i);
		break;
	}
	// erase the option itself
	for (vector<Option *>::iterator j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_key() == key) {
		delete * j;
		opt_list.erase(j);
		break;
	}
}

void Parser::remove(const std::string & name)
{
	// erase help first (have to leave the extra help lines.)
	for (vector<HelpLine>::iterator i = help_list.begin(); i != help_list.end(); i ++) if (i->opt && i->opt->get_name() == name) {
		help_list.erase(i);
		break;
	}
	// erase the option itself
	for (vector<Option *>::iterator j = opt_list.begin(); j != opt_list.end(); j ++) if ((* j)->get_name() == name) {
		delete * j;
		opt_list.erase(j);
		break;
	}
}

void Parser::remove_all()
{
	help_list.clear();
	while (opt_list.size()) {
		delete opt_list.back();
		opt_list.pop_back();
	}
}

string Parser::get_help()
{
	string h;
	for (vector<HelpLine>::iterator i = help_list.begin(); i != help_list.end(); i ++) {
		h += i->msg;
		if (i->opt) {
			h += i->opt->get_help();
		}
		h += '\n';
	}
	return h;
}

namespace { // local callback functions
	bool help_callback(int key, const string & str, void * data)
	{
		Parser * p = static_cast<Parser *>(data);
		cout << p->get_header();
		cout << '\n';
		cout << p->get_help();
		cout << '\n';
		exit(0);
	}

	bool version_callback(int key, const string & str, void * data)
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
			vector<Option *>::iterator j = opt_list.begin();
			while (true) {
				if (j == opt_list.end()) throw UnknError(name);
				if ((* j)->get_name() == name) break;
				j ++;
			}
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
	for (vector<HelpLine>::iterator i = help_list.begin(); i != help_list.end(); i ++) {
		h += i->msg;
		if (i->opt) {
			h += i->opt->get_help(Option::HF_NODASH);
		}
		h += '\n';
	}
	return h;
}

namespace { // local callback functions
	bool sub_help_callback(int key, const string & str, void * data)
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

Error::Error()
{
}

Error::Error(const string & m)
{
	msg = m;
}

string Error::get_msg()
{
	return msg;
}

OptError::OptError(const string & o)
{
	opt = o;
	msg = "error processing option: " + opt;
}

OptError::OptError(const string & o, const string & m)
{
	opt = o;
	msg = m + " for option: " + o;
}

ConvError::ConvError(const string & str, const string & type)
{
	msg = "error converting '" + str + "' to " + type;
}

UnknError::UnknError(const string & o)
{
	msg = "unknown option: " + o;
}
