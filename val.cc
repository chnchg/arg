/* val.cc
 *
 * Copyright (C) 2010,2018 Chun-Chung Chen <cjj@u.washington.edu>
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

#include "val.hh"
#include <iostream>
#include <cstdlib>

using namespace arg;
using namespace std;

SetValue::Element::Element(std::string const & name, int value, std::string const & help) :
	name(name),
	value(value),
	help(help)
{}

SetValue::SetValue(int & v) :
	var(v),
	help_default(false)
{
}

void SetValue::add_help(string const & title)
{
	help_title = title;
	help_default = true;
	add("help", "show this list");
}

void SetValue::add_help(string const & title, int value)
{
	help_title = title;
	help_default = true;
	add("help", value, "show this list");
}

void SetValue::add(string const & name, string const & help)
{
	int value = - 100;
	for (auto & i: set_list) {
		if (name == i.name) throw Error("duplicated element in SetValue");
		if (value >= i.value) value = i.value - 1;
	}
	set_list.emplace_back(name, value, help);
}

void SetValue::add(string const & name, int value, string const & help)
{
	for (auto & i: set_list) {
		if (name == i.name) throw Error("duplicated element in SetValue");
		if (value == i.value) throw Error("duplicated value in SetValue");
	}
	set_list.emplace_back(name, value, help);
}

void SetValue::set(string const & str)
{
	if (help_default && str == "help") { // print help and quit
		cout << '\n';
		cout << help_title << '\n';
		cout << '\n';
		cout << get_help();
		cout << '\n';
		exit(0);
	}
	for (auto & i: set_list) {
		if (str == i.name) {
			var = i.value;
			return;
		}
	}
	throw ConvError(str, "an element in SetValue");
}

string SetValue::to_str() const
{
	for (auto & i: set_list) {
		if (var == i.value) return i.name;
	}
	throw Error("no such value in Set");
}

string SetValue::get_type() const
{
	return "set()";
}

int SetValue::get_value(string const & name) const
{
	for (auto & i: set_list) {
		if (name == i.name) return i.value;
	}
	throw Error("element '" + name + "' not found in SetValue");
}

string const & SetValue::get_name(int value) const
{
	for (auto & i: set_list) {
		if (value == i.value) return i.name;
	}
	ostringstream os;
	os << "value '" << value << "' not found in SetValue";
	throw Error(os.str());
}

string const & SetValue::get_help(string const & name) const
{
	for (auto & i: set_list) {
		if (name == i.name) return i.help;
	}
	throw Error("element '" + name + "' not found in SetValue");
}

string const & SetValue::get_help(int value) const
{
	for (auto & i: set_list) {
		if (value == i.value) return i.help;
	}
	ostringstream os;
	os << "value '" << value << "' not found in SetValue";
	throw Error(os.str());
}

string SetValue::get_help() const
{
	string help;
	for (auto & i: set_list) {
		string s = "    ";
		s += i.name;
		s += ": ";
		if (s.size() < 16) s.resize(16, ' ');
		s += i.help;
		s += '\n';
		help += s;
	}
	return help;
}

TermValue::TermValue(std::string & v) :
	var(v),
	help_default(false)
{
}

void TermValue::add_help(string const & title)
{
	help_title = title;
	help_default = true;
	add("help", "show this list");
}

void TermValue::add(string const & name, string const & help)
{
	for (auto e: term_list) if (name == e.name) throw Error("duplicated element in SetValue");
	Element e;
	e.name = name;
	e.help = help;
	term_list.push_back(e);
}

void TermValue::set(string const & str)
{
	if (help_default && str == "help") { // print help and quit
		cout << '\n';
		cout << help_title << '\n';
		cout << '\n';
		cout << get_help();
		cout << '\n';
		exit(0);
	}
	for (auto e: term_list) if (str == e.name) {
		var = str;
		return;
	}
	throw ConvError(str, "an element in SetValue");
}

string TermValue::to_str() const
{
	for (auto e: term_list) if (var == e.name) return var;
	throw Error("no such value in Set");
}

string TermValue::get_type() const
{
	return "term()";
}

string const & TermValue::get_help(string const & name) const
{
	for (auto & e: term_list) if (name == e.name) return e.help;
	throw Error("element '" + name + "' not found in SetValue");
}

string TermValue::get_help() const
{
	string help;
	for (auto e: term_list) {
		string s = "    ";
		s += e.name;
		s += ": ";
		if (s.size() < 16) s.resize(16, ' ');
		s += e.help;
		s += '\n';
		help += s;
	}
	return help;
}

RelValue::RelValue(double & var, bool & is_relative) :
	v(var),
	rel(is_relative)
{
}

void RelValue::set(string const & str)
{
	istringstream i;
	if (str[0] == '+') { // relative value
		rel = true;
		i.str(str.substr(1));
	}
	else {
		rel = false;
		i.str(str);
	}
	i >> v;
}

string RelValue::to_str() const
{
	ostringstream o;
	if (rel) o << '+';
	o << v;
	return o.str();
}

string RelValue::get_type() const
{
	return "relative_double";
}
