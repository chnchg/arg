/* arg.hh
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

#ifndef ARG_HH
#define ARG_HH 1
#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
namespace arg {
	// proxy to values of command line options, need to know where to store the values
	class Value
	{
	public:
		virtual ~Value();
		virtual void set(const std::string & str); // convert the str to value and put it in storage
		virtual std::string to_str() const; // convert the value to a string
	};

	// signature for callback functions
	typedef bool (CallBack)(int, const std::string & , void *);

	class Option
	{
		int key;
		std::string name;

		Value * store_ptr; // pointer to storage space
		bool store_optional; // if value string is optional
		std::string store_str; // default value string

		int * set_var; // variable to set
		int set_value; // value to set
		bool set_once; // if can only set once
		int set_init; // initial value, 

		CallBack * call_func; // callback function
		void * call_data; // data to pass to callback function

		std::string help_text;
		std::string help_var;
		bool help_default; // whether to show default value of store
	public:
		Option(int key, const std::string & name);
		~Option();

		// option modifiers
		template<typename T> Option & stow(T & t); // stow value to streamable variable
		Option & store(Value * ptr = 0); // store value to "* ptr", the Value will be released by the Option
		Option & optional(const std::string & str = ""); // value is optional defaulting to "str"
		Option & set(int * var, int value = - 1); // set "* var" to "value"
		Option & once(int init = 0); // can only be set once, with distinct value, "init"
		Option & call(CallBack * func, void * data); // call function "* func" with "data" as extra argument
		Option & help(const std::string & text, const std::string & var = ""); // help text
		Option & help_word(const std::string & var);
		Option & show_default(bool do_show = true);

		bool take_value();
		bool need_value();
		int get_key();
		const std::string & get_name();

		enum HelpFormat {
			HF_REGULAR,
			HF_NODASH
		};
		std::string get_help(HelpFormat format = HF_REGULAR);

		void process();
		void process(const std::string & str);
	};

	class Parser
	{
		std::string header_text;
		std::string version_info;
	protected:
		std::vector<Option *> opt_list;
		std::vector<std::string> arg_list;
		struct HelpLine {
			std::string msg;
			Option * opt;
		};
		std::vector<HelpLine> help_list;
	public:
		~Parser();
		void add_help(const std::string & msg);
		Option & add_opt(int key, const std::string & name = "", bool hide = false);
		Option & add_opt(const std::string & name, bool hide = false);
		std::vector<std::string> & args();
		void parse(int argc, char * argv[], bool ignore_unknown = false);
		void set_header(const std::string & text);
		const std::string & get_header() const;

		// find existing options
		Option * find(int key);
		Option * find(const std::string & name);

		// removing options
		void remove(int key);
		void remove(const std::string & name);
		void remove_all(); // remove all options

		std::string get_help();
		// default options
		Option & add_opt_help();
		Option & add_opt_version(const std::string & version);
	};

	class SubParser :
		public Value,
		public Parser
	{
		char sep;
	public:
		SubParser();
		void set(const std::string & str); // parse the str
		std::string get_help();
		void set_sep(char s); // set the separator to s from ','
		// default options
		Option & add_opt_help();
	};

	// Errors:

	class Error
	{
	protected:
		std::string msg;
	public:
		Error();
		Error(const std::string & msg);
		std::string get_msg();
	};

	class OptError : // option processing error
		public Error
	{
	protected:
		std::string opt;
	public:
		OptError(const std::string & opt);
		OptError(const std::string & opt, const std::string & msg);
	};

	class ConvError : // conversion error
		public Error
	{
	public:
		ConvError(const std::string & str, const std::string & type);
	};

	class UnknError : // unknow option error
		public Error
	{
	public:
		UnknError(const std::string & opt);
	};

	// Templates:

	// value types that have << and >> defined for istream/ostream
	template <typename T>
	class StreamableValue :
		public Value
	{
		T & ptr;
	public:
		StreamableValue(T & t) :
			ptr(t)
		{
		}

		void set(const std::string & str)
		{
			std::istringstream s(str);
			T tmp;
			s >> tmp;
			if (s.bad() || ! s.eof()) throw ConvError(str, typeid(T).name());
			ptr = tmp;
		}

		std::string to_str() const
		{
			std::ostringstream s;
			s << ptr;
			return s.str();
		}
	};

	template<typename T>
	Option & Option::stow(T & t)
	{
		return store(new StreamableValue<T>(t));
	}
}
#endif // ARG_HH
