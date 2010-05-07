/* val.hh
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

// This header file provides additional Value classes:
//
//   SetValue:  value represents a choice from a set of names
//   ListValue: a list of comma-separated values
//   RelValue:  value of double that can be absolute or relative

#ifndef VAL_HH
#define VAL_HH 1
#include <arg.hh>
#include <vector>
#include <sstream>
namespace arg {
	// Extensions:

	// Set of names in strings
	class SetValue :
		public Value
	{
		int & var;
		std::string help_title;
		bool help_default;
		struct Element {
			std::string name;
			int value;
			std::string help;
		};
		std::vector<Element> set_list;
	public:
		SetValue(int & var);
		void add_help(const std::string & title = "Available values:");
		void add_help(const std::string & title, int value);
		void add(const std::string & name, const std::string & help = "");
		void add(const std::string & name, int value, const std::string & help = "");

		void set(const std::string & str);
		std::string to_str() const;

		// additional access to set
		int get_value(const std::string & name) const;
		const std::string & get_name(int value) const;
		const std::string & get_help(const std::string & name) const;
		const std::string & get_help(int value) const;
		std::string get_help() const;
	};

	// List of values seperated by comma
	template <typename T>
	class ListValue :
		public Value
	{
		std::vector<T> & plist;
		char sep;
	public:
		ListValue(std::vector<T> & list, char seperator = ',') :
			plist(list),
			sep(seperator)
		{
		}

		void set(const std::string & str)
		{
			plist.clear();
			std::string::size_type n = 0;
			while (n < str.length()) {
				std::string::size_type m = str.find(sep, n);
				if (m == std::string::npos) m = str.length();
				std::istringstream i(str.substr(n, m - n));
				T v;
				i >> v;
				plist.push_back(v);
				n = m + 1;
			}
		}

		std::string to_str() const
		{
			std::ostringstream o;
			for (typename std::vector<T>::const_iterator i = plist.begin(); i != plist.end(); i ++) {
				if (i != plist.begin()) o << sep;
				o << * i;
			}
			return o.str();
		}
	};

	// double that can be relative (if it begins with '+' sign)
	class RelValue :
		public Value
	{
		double & v;
		bool & rel;
	public:
		RelValue(double & var, bool & is_relative);
		void set(const std::string & str);
		std::string to_str() const;
	};
}
#endif // VAL_HH
