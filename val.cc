/* val.cc
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

#include <val.hh>
#include <iostream>
#include <cstdlib>

using namespace arg;
using namespace std;

SetValue::SetValue(int & v) :
	var(v),
	help_default(false)
{
}

void SetValue::add_help(const string & title)
{
	help_title = title;
	help_default = true;
	add("help", "show this list");
}

void SetValue::add_help(const string & title, int value)
{
	help_title = title;
	help_default = true;
	add("help", value, "show this list");
}

void SetValue::add(const string & name, const string & help)
{
	int value = - 100;
	for (vector<Element>::iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (name == i->name) throw Error("duplicated element in SetValue");
		if (value >= i->value) value = i->value - 1;
	}
	Element e;
	e.name = name;
	e.value = value;
	e.help = help;
	set_list.push_back(e);
}

void SetValue::add(const string & name, int value, const string & help)
{
	for (vector<Element>::iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (name == i->name) throw Error("duplicated element in SetValue");
		if (value == i->value) throw Error("duplicated value in SetValue");
	}
	Element e;
	e.name = name;
	e.value = value;
	e.help = help;
	set_list.push_back(e);
}

void SetValue::set(const string & str)
{
	if (help_default && str == "help") { // print help and quit
		cout << '\n';
		cout << help_title << '\n';
		cout << '\n';
		cout << get_help();
		cout << '\n';
		exit(0);
	}
	for (vector<Element>::iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (str == i->name) {
			var = i->value;
			return;
		}
	}
	throw ConvError(str, "an element in SetValue");
}

string SetValue::to_str() const
{
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (var == i->value) return i->name;
	}
	throw Error("no such value in Set");
}

int SetValue::get_value(const string & name) const
{
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (name == i->name) return i->value;
	}
	throw Error("element '" + name + "' not found in SetValue");
}

const string & SetValue::get_name(int value) const
{
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (value == i->value) return i->name;
	}
	ostringstream os;
	os << "value '" << value << "' not found in SetValue";
	throw Error(os.str());
}

const string & SetValue::get_help(const string & name) const
{
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (name == i->name) return i->help;
	}
	throw Error("element '" + name + "' not found in SetValue");
}

const string & SetValue::get_help(int value) const
{
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		if (value == i->value) return i->help;
	}
	ostringstream os;
	os << "value '" << value << "' not found in SetValue";
	throw Error(os.str());
}

string SetValue::get_help() const
{
	string help;
	for (vector<Element>::const_iterator i = set_list.begin(); i != set_list.end(); i ++) {
		string s = "    ";
		s += i->name;
		s += ": ";
		if (s.size() < 16) s.resize(16, ' ');
		s += i->help;
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

void RelValue::set(const string & str)
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
