/* arg.hh
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

#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include <memory>
namespace arg {
	/// proxy to values of command line options, need to know where to store the values
	class Value
	{
	public:
		virtual ~Value();
		virtual void set(std::string const & str); ///<convert the str to value and put it in storage
		virtual std::string to_str() const; ///<convert the value to a string
		virtual std::string get_type() const; ///<type name of the value
	};

	/// signature for callback functions
	typedef bool (CallBack)(int, std::string const &, void *);

	/// options to be parsed
	class Option
	{
		int key;
		std::string name;

		std::shared_ptr<Value> store_ptr; ///<pointer to storage space
		bool store_optional; ///<if value string is optional
		std::string store_str; ///<default value string

		bool * set_bool;
		bool bool_value;
		int * set_var; ///<variable to set
		int set_value; ///<value to set
		bool set_once; ///<if can only set once
		int set_init; ///<initial value, 

		CallBack * call_func; ///<callback function
		void * call_data; ///<data to pass to callback function

		std::string help_text;
		std::string help_var;
		bool help_default; ///<whether to show default value of store
	public:
		/// command-line option with key and name
		Option(
			int key, ///< unique single character key for the option
			std::string const & name ///<name for the option
		);
		~Option();

		// option modifiers
		template<typename T> Option & stow(T & t); ///<stow value to streamable variable
		Option & store(std::shared_ptr<Value> ptr = 0); ///<store value to "* ptr", the Value will be released by the Option
		Option & optional(std::string const & str = ""); ///<value is optional defaulting to "str"
		Option & set(int * var, int value = - 1); ///<set "* var" to "value"
		Option & set(bool & var, bool value = true); ///<set "* var" to "value"
		Option & once(int init = 0); ///<can only be set once, with distinct value, "init"
		Option & call(CallBack * func, void * data); ///<call function "* func" with "data" as extra argument
		Option & help(std::string const & text, std::string const & var = ""); ///<help text
		Option & help_word(std::string const & var); ///<help word
		Option & show_default(bool do_show = true); ///<show default value in help

		bool take_value();
		bool need_value();
		int get_key();
		std::string const & get_name();

		enum HelpFormat {
			HF_REGULAR,
			HF_NODASH
		};
		std::string get_help(HelpFormat format = HF_REGULAR);

		void process();
		void process(std::string const & str);
	};

	/// Positional arguments on command line
	class Argument
	{
		std::string name;
		std::shared_ptr<Value> store_ptr; ///<pointer to storage space
		std::string help_text;
	public:
		/// positional argument with name
		Argument(
			std::string const & name ///<name for the argument
		);
		~Argument();

		// option modifiers
		template<typename T> Argument & stow(T & t); ///<stow value to streamable variable
		Argument & store(std::shared_ptr<Value> ptr = 0); ///<store value to "* ptr", the Value will be released by the Argument
		Argument & help(std::string const & text); ///<help text

		std::string const & get_name(); ///<get name of the argument

		enum HelpFormat {
			HF_REGULAR,
			HF_NODASH
		};
		std::string get_help(); ///<get help text
		void process(std::string const & str); ///<process string data
	};

	/// The command-line parser
	class Parser
	{
		std::string header_text;
		std::string version_info;
	protected:
		std::string prog_name; ///<name to identify the program
		std::vector<std::shared_ptr<Option>> opt_list;
		std::vector<std::shared_ptr<Argument>> arg_list;
		std::vector<std::string> arg_strs;
		struct HelpLine {
			std::string msg;
			std::shared_ptr<Option> opt;
			HelpLine(std::string const & m, std::shared_ptr<Option> o);
		};
		std::vector<HelpLine> help_list;
	public:
		~Parser();
		void add_help(std::string const & msg); ///<add additional help text between option helps
		Option & add_opt(int key, std::string const & name = "", bool hide = false); ///<add an Option
		Option & add_opt(std::string const & name, bool hide = false); ///<add an Option without a specified key
		Option & get_opt(std::string const & name); ///<get an existing Option
		std::vector<std::string> & args(); ///<get the argument list
		/// perform command-line parsing
		void parse(
			int argc, ///<count of command-line tokens
			char * argv[], ///<c-string array of command-line tokens
			bool ignore_unknown = false ///<whether to ignore unknown options
		);
		void set_header(std::string const & text); ///<set the header in help
		std::string const & get_header() const; ///<get the header text of help

		// find existing options
		std::shared_ptr<Option> find(int key);
		std::shared_ptr<Option> find(std::string const & name);

		// removing options
		void remove(int key);
		void remove(std::string const & name);
		void remove_all(); // remove all options

		std::string get_help();
		// default options
		Option & add_opt_help();
		Option & add_opt_version(std::string const & version);

		// positional arguments
		Argument & add_arg(std::string const & name);
	};

	/// A Parser that is a Value itself
	class SubParser :
		public Value,
		public Parser
	{
		char sep;
	public:
		SubParser();
		void set(std::string const & str); // parse the str
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
		Error(std::string const & msg);
		std::string get_msg();
	};

	class OptError : // option processing error
		public Error
	{
	protected:
		std::string opt;
	public:
		OptError(std::string const & opt);
		OptError(std::string const & opt, std::string const & msg);
	};

	class ConvError : // conversion error
		public Error
	{
	public:
		ConvError(std::string const & str, std::string const & type);
	};

	class UnknError : // unknow option error
		public Error
	{
	public:
		UnknError(std::string const & opt);
	};

	class MissingError : // missing argument
		public Error
	{
	public:
		MissingError(const  std::string & type);
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

		void set(std::string const & str)
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

		std::string get_type() const
		{
			return typeid(T).name();
		}
	};

	template<typename T>
	Option & Option::stow(T & t)
	{
		return store(std::make_shared<StreamableValue<T>>(t));
	}

	template<typename T>
	Argument & Argument::stow(T & t)
	{
		return store(std::make_shared<StreamableValue<T>>(t));
	}
}
