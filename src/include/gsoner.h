#pragma once
#include <list>
#include <map>
#include <vector>
#include <assert.h>
#include <xutility>
#include <iostream>

namespace acl
{
	namespace gson
	{

	class gsoner
	{
	public:
		gsoner ()
		{
			status_ = e_uninit;
			gen_header_ = NULL;
			gen_source_ = NULL;
			default_ = true;
		}
#define GSON_EXCEPTION(E)\
		struct E##:std::exception\
		{\
			E(){}\
			E(const char *msg):exception(msg){}\
		};

		GSON_EXCEPTION(syntax_error);
		GSON_EXCEPTION(unsupported_type);
		struct field_t 
		{
			enum type_t
			{
				e_bool,
				e_bool_ptr,
				e_number,
				e_double,
				e_cstr,//char *
				e_ccstr,//const char *
				e_string,
				e_list,
				e_vector,
				e_map,
				e_object,
			};
			type_t type_;
			std::string name_;
			bool required_;
			field_t()
			{

			}
			field_t(type_t t, const std::string &name,bool required)
				:type_(t),
				name_(name),
				required_(required)
			{

			}
			virtual ~field_t()
			{

			}
		};
		struct object_t
		{
			typedef std::list<field_t> fields_t;
			fields_t fields_;
			std::string name_;
			void reset ()
			{
				fields_.clear ();
				name_.clear ();
			}
		};

		struct function_code_t
		{
			std::string declare_;
			std::string declare_ptr_;
			std::string declare2_;
			std::string definition_;
			std::string definition_ptr_;
			std::string definition2_;
		};
		std::string get_node_func (const field_t &field)
		{
			acl::string code;
			switch (field.type_)
			{
				case gsoner::field_t::e_bool:
				case field_t::e_bool_ptr:
					return "add_bool";
				case gsoner::field_t::e_number:
					return "add_number";
				case gsoner::field_t::e_double:
					return "add_double";
				case gsoner::field_t::e_string:
				case gsoner::field_t::e_cstr:
				case gsoner::field_t::e_ccstr:
					return "add_text";
				case gsoner::field_t::e_list:
				case gsoner::field_t::e_vector:
				case gsoner::field_t::e_map:
				case gsoner::field_t::e_object:
					return "add_child";

				default:
					break;
			}
			return "error_type";
		}
		std::string get_gson_func_laber (const field_t &field)
		{
			acl::string code;
			switch (field.type_)
			{
				case gsoner::field_t::e_list:
				case gsoner::field_t::e_vector:
				case gsoner::field_t::e_map:
				case gsoner::field_t::e_object:
					return "acl::gson::gson(json,";
				default:
					return "acl::gson::get_value(";
			}
			return "error_type";
		}

		function_code_t gen_pack_code (const object_t &obj)
		{

			function_code_t code;
			std::string str;
			str += "acl::json_node& gson (acl::json & json, const ";
			str += obj.name_;
			
			code.declare_ptr_ = str;
			code.declare_ptr_ += " *obj);";
			code.declare2_ = "acl::string gson(const ";
			code.declare2_ += obj.name_;
			code.declare2_ += " &obj);";
			code.definition2_ = 
				code.declare2_.substr (0, code.declare2_.find (";"));
			code.definition2_ += "\n{\n"
				"    acl::json json;\n"
				"    acl::json_node &node = acl::gson::gson (json, obj);\n"
				"    return node.to_string ();\n}\n\n";

			code.definition_ptr_ = str;
			code.definition_ptr_ += "*obj)\n"
				"{\n"
				"    return gson (json, *obj);\n"
				"}\n\n";
			str += " &obj)";
			
			code.declare_ = str;
			code.declare_ += ";";

			str += "\n{\n";
			str += tab_;
			str += "acl::json_node &node =  json.create_node();\n";
			
			for (object_t::fields_t::const_iterator 
				 itr = obj.fields_.begin ();
				itr != obj.fields_.end(); ++itr)
			{
				str += tab_;
				str += "node.";
				str += get_node_func (*itr);
				str += "(\"";
				str += itr->name_;
				str += "\", ";
				str += get_gson_func_laber (*itr);
				str += "obj.";
				str += itr->name_;
				str += "));\n";
			}
			str += "\n";
			str += tab_;
			str += "return node;\n}";

			code.definition_ = str;
			return code;
		}

		std::string get_unpack_code(const std::string &obj_name, const field_t &field)
		{
			if((field.type_ == field_t::e_bool ||
			   field.type_ == field_t::e_bool_ptr ||
			   field.type_ == field_t::e_number ||
			   field.type_ == field_t::e_ccstr ||
			   field.type_ == field_t::e_cstr ||
			   field.type_ == field_t::e_double ||
			   field.type_ == field_t::e_string) &&
			   field.required_)

				return tab_
				+ "if(!" + field.name_ + " ||"
				+ "!(result = gson(*" + field.name_ + ",&obj."
				+ field.name_ + "), result.first))\n" + tab_ + tab_
				+ "return std::make_pair(false,\"required ["
				+ obj_name + "." + field.name_
				+ "] failed:{\"+result.second+\"}\");";

			else if(field.required_)
				return tab_
				+ "if(!" + field.name_ + " ||" + "!" + field.name_
				+ "->get_obj()||" + "!(result = gson(*" + field.name_
				+ "->get_obj(), &obj." + field.name_ + "), result.first))\n"
				+ tab_ + tab_ + "return std::make_pair(false, \"required ["
				+ obj_name + "." + field.name_
				+ "] failed:{\"+result.second+\"}\");";

			else if((field.type_ == field_t::e_bool ||
					field.type_ == field_t::e_bool_ptr ||
					field.type_ == field_t::e_ccstr ||
					field.type_ == field_t::e_cstr ||
					field.type_ == field_t::e_double ||
					field.type_ == field_t::e_string) &&
					field.required_ == false)

				return tab_ +
				"if(" + field.name_ + ")\n" + tab_ + tab_ +
				"gson(*" + field.name_ + ",&obj." + field.name_ + ");\n";

			else if(field.required_ == false)
				return tab_
				+ "if(" + field.name_ + "&& " + field.name_
				+ "->get_obj())\n" + tab_ + tab_ + " gson(*" + field.name_
				+ "->get_obj(), &obj." + field.name_ + ");\n";

			return "unknown_type";
		}
		std::string get_node_name(const std::string &name)
		{
			return std::string(tab_+"acl::json_node *")
				+ name + " = node[\""+name + "\"];";
		}

		function_code_t gen_unpack_code(const object_t &obj)
		{
			std::list<std::string >node_names;
			std::list<std::string >unpack_codes;
			for (std::list<field_t>::const_iterator itr = 
				 obj.fields_.begin(); 
				 itr!= obj.fields_.end();++itr)
			{
				node_names.push_back(get_node_name(itr->name_));
				unpack_codes.push_back(get_unpack_code(obj.name_, *itr));
			}
			function_code_t code;
			std::string prefix = 
				"std::pair<bool,std::string> gson(acl::json_node &node, ";
			code.declare_ =  prefix + obj.name_ + " &obj);";

			code.definition_ += prefix;
			code.definition_ += obj.name_ + " &obj)\n{\n";
			
			for (std::list<std::string>::iterator itr = node_names.begin();
				 itr != node_names.end();++itr)
			{
				code.definition_ += *itr;
				code.definition_ += "\n";
			}
			code.definition_ += tab_ + "std::pair<bool, std::string> result;\n\n";
			for(std::list<std::string>::iterator itr = unpack_codes.begin();
				itr != unpack_codes.end(); ++itr)
			{
				code.definition_ += *itr;
				code.definition_ += "\n \n";
			}
			code.definition_ += tab_ + "return std::make_pair(true,\"\");\n}\n\n";

			code.definition_ptr_ += prefix + obj.name_ + " *obj)";
			code.definition_ptr_ += "\n{\n" + tab_ + "return gson(node, *obj);\n}\n\n";

			return code;
		}
		std::string test_pack ()
		{
			object_t obj;
			obj.name_ = "user_t";
			obj.fields_.push_back (field_t{field_t::e_bool,"is_stop", false});

			function_code_t code = gen_pack_code (obj);

			printf (code.declare_.c_str ());
			printf (code.definition_.c_str ());
			return code.declare_;
		}

		enum code_parser_status_t
		{
			e_uninit,
			e_comment,
			e_struct_begin,
			e_struct_end,
		};
		/*
			struct user_t
			{
			
			}
		*/
		//namespace
		bool check_namespace()
		{
			std::string temp = codes_.substr(pos_, strlen("namespace"));
			if (temp == "namespace")
			{
				pos_ += strlen("namespace");
				namespaces_.push_back(get_name());
				return true;
			}
			return false;
		}
		bool check_namespace_end()
		{
			if (namespaces_.size())
			{
				namespaces_.pop_back();
				return true;
			}
			return false;
		}
		std::string get_name()
		{
			std::string name;
			skip_space();
			while(codes_[pos_] != ' ' &&
				  codes_[pos_] != '{' &&
				  codes_[pos_] != '\r'  &&
				  codes_[pos_] != '\n' &&
				  codes_[pos_] != '\t'&&
				  codes_[pos_] != '/') //
			{
				name.push_back(codes_[pos_]);
				pos_++;
			}
		check_again:
			skip_space();
			if(codes_[pos_] == '/')
			{
				if(check_comment() == false)
					throw syntax_error();
				goto check_again;
			}
			if(codes_[pos_] == '{')
			{
				pos_++;
			}
			return name;
		}
		std::string get_namespace()
		{
			std::string result;
			for (std::list<std::string>::iterator itr = namespaces_.begin();
				 itr != namespaces_.end(); ++itr)
			{
				result += *itr;
				result += "::";
			}
			return result;
		}
		bool check_struct_begin ()
		{
			if (status_ != e_uninit)
				return false;
			std::string tmp = codes_.substr(pos_, strlen("struct"));
			//struct user_t
			if (tmp == "struct")
			{
				pos_ += strlen("struct");
				status_ = e_struct_begin;
				current_obj_.name_ = get_namespace() + get_name();
				if (codes_[pos_] == ';')
				{
					pos_++;
					//struct user_t ; end of struct;
					status_ = e_uninit;
				}
				return true;
			}
			//not struct block
			return false;
		}
		bool check_struct_end ()
		{
			if (status_ == e_struct_begin)
			{
				pos_++;
				try_skip_comment();
				if (codes_[pos_] == ';')
				{
					pos_++;
					if (current_obj_.name_.size())
					{
						objs_.push_back (current_obj_);
						current_obj_.reset ();
					}
					status_ = e_uninit;
				}
				return true;
			}
			return false;
		}
		void try_skip_comment()
		{
		again:
			skip_space();
			char ch = codes_[pos_];
			if(ch == '/')
			{
				if(check_comment())
					goto again;
			}
		}
		bool check_include()
		{
			std::string tmp = codes_.substr(pos_, strlen("#include"));
			if (tmp == "#include")
			{
				pos_ += strlen("#include");
				try_skip_comment();
				char sym = codes_[pos_++];
				if(sym == '<')
					sym = '>';
				std::string include;
				while (codes_[pos_] != sym)
				{
					include.push_back(codes_[pos_]);
					pos_++;
				}
				pos_++;
				includes_.push_back(include);
				return true;
			}
			return false;
		}
		bool check_comment ()
		{
			std::string commemt;
			bool result = false;
			if (codes_[pos_] == '/' &&
				codes_[pos_ + 1] == '/')
			{
				result = true;
				pos_++;
				pos_++;
				//skip a line
				while(codes_[pos_] != '\n')
				{
					commemt.push_back(codes_[pos_]);
					pos_++;
				}
			}
			else if (codes_[pos_] == '/' &&
					 codes_[pos_ + 1] == '*')
			{
				result = true;
				//skip /**/comment
				pos_++;
				pos_++;
				while(codes_[pos_] != '*' ||
					  codes_[pos_ + 1] != '/')
				{
					commemt.push_back(codes_[pos_]);
					pos_++;
				}
				pos_++;
				pos_++;
			}
			if (result)
			{
				if(commemt.find("Gson@optional") == std::string::npos)
					required_ = false;
				else if(commemt.find("Gson@required") == std::string::npos)
					required_ = true;
			}
			return result;
		}
		void skip_space ()
		{
			while (codes_[pos_] == ' '||
				   codes_[pos_] == '\r' ||
				   codes_[pos_] == '\n' ||
				   codes_[pos_] == '\t')
				pos_++;
		}
		//check the code is struct member.
		std::pair<bool, std::string>
			get_function_declare()
		{
			if(status_ != e_struct_begin)
				return std::make_pair(false, "");

			int j = pos_;
			std::string lines;
			skip_space();
			while(true)
			{
				if(codes_[j] == '/')
				{
					if(check_comment() == false)
						throw syntax_error();
					continue;
				}
				if(codes_[j] == ';')
					break;
				if(codes_[j] == '(')
					break;
				lines.push_back(codes_[j]);
				j++;
			}
			if(codes_[j] == ';')
			{
				//not function, maybe member field
				return std::make_pair(false, "");
			}
			lines.push_back('(');
			j++;
			int syn = 1;
			while(true)
			{
				if(codes_[j] == '/')
				{
					if(check_comment() == false)
						throw syntax_error();
					continue;
				}
				{
					if(codes_[j] == ')')
						syn--;
					if(syn == 0)
					{
						lines.push_back(codes_[j]);
						break;
					}
				}
				if(codes_[j] == '(')
					syn++;
				lines.push_back(codes_[j]);
				j++;
			}
			j++;
			pos_ = j;
			return std::make_pair(true,lines);
		}
		bool check_function()
		{
			if(status_ != e_struct_begin)
				return false;
			std::pair<bool, std::string> res = get_function_declare();
			if(res.first == false)
				return false;

			return true;
		}
		bool check_member()
		{
			//struct user_t{int id;  
			
			if (status_ == e_struct_begin)
			{
				required_ = default_;
				std::string lines;
				skip_space ();
				while (true)
				{
					if(codes_[pos_] == '/')
					{
						if(check_comment() == false)
							throw syntax_error();
						continue;
					}
					if(codes_[pos_] == ';')
						break;
					lines.push_back (codes_[pos_]);
					pos_++;
				}
				//skip ;
				pos_++;
				std::string name;
				std::string types;
				//remove back spacce. lilke "int a    ; "
				int e = lines.size () - 1;
				while (lines[e] == ' ' ||
					   lines[e] == '\r' ||
					   lines[e] == '\n' ||
					   lines[e] == '\t')
					e--;

				while (lines[e] != ' ' &&
					   lines[e] != '\r' &&
					   lines[e] != '\n' &&
					   lines[e] != '\t' &&
					   lines[e] != '*' &&
					   lines[e] != '&')
				{
					name.push_back (lines[e]);
					e--;
				}
				//get name
				std::reverse (name.begin (), name.end ());

				types = lines.substr (0,e+1);
				std::list<std::string> tokens;
				std::string token;
				for (std::string::iterator itr = types.begin ();
					itr!= types.end();++itr)
				{
					if (*itr == ' '||
						*itr == '\r'||
						*itr == '\n'||
						*itr == '\t')
					{
						if (token.size ())
						{
							tokens.push_back (token);
							token.clear ();
						}
					}
					else if(*itr == '*'||
							*itr == '&')
					{
						if(token.size())
						{
							tokens.push_back(token);
							token.clear();
						}
						if(*itr == '*')
							tokens.push_back("*");
						else
							tokens.push_back("&");
					}
					else if(*itr == ':')
					{
						if(token.size())
						{
							if (token.back() == ':')
							{
								tokens.push_back("::");
								token.clear();
								continue;
							}
							else
							{
								if(token.size())
								{
									tokens.push_back(token);
									token.clear();
								}
							}
						}
						
						token.push_back(':');
					}
					else if (*itr == '<')
					{
						if(token.size())
						{
							tokens.push_back(token);
							token.clear();
						}
						tokens.push_back("<");
					}
					else if(*itr == '>')
					{
						if(token.size())
						{
							tokens.push_back(token);
							token.clear();
						}
						tokens.push_back(">");
					}
					else if(*itr == ',')
					{
						if(token.size())
						{
							tokens.push_back(token);
							token.clear();
						}
						tokens.push_back(",");
					}
					else
					{
						token.push_back (*itr);
					}
				}
				if (token.size ())
					tokens.push_back (token);
				if (tokens.size() == 0)
				{
					printf ("\"%s\"[syntax error]", name.c_str ());
					assert (false);
				}

				//std    :: list <int> a;
				std::string first = tokens.front();
				if(first == "const")
				{
					tokens.pop_front();
					std::string first = tokens.front();
					if(tokens.empty())
						throw unsupported_type(("unsupported \"" + lines+"\"").c_str());
				}
				if (first.find("char") != std::string::npos)
				{
					if(first == "char*")
					{
						field_t f;
						f.name_ = name;
						f.required_ = required_;
						f.type_ = field_t::e_cstr;
						current_obj_.fields_.push_back(f);
						return true;
					}
					tokens.pop_front();
					if (tokens.size())
					{
						first = tokens.front();
						if (first == "*")
						{
							field_t f;
							f.name_ = name;
							f.required_ = required_;
							f.type_ = field_t::e_cstr;
							current_obj_.fields_.push_back(f);
							return true;
						}
					}
					throw unsupported_type("unsupported 'char' type");
				}
				if (first == "std")
				{
					for (std::list<std::string>::iterator itr = tokens.begin ();
						itr != tokens.end();++itr)
					{
						if (itr->find ("string") != std::string::npos)
						{
							field_t f;
							f.name_ = name;
							f.required_ = required_;
							f.type_ = field_t::e_string;
							current_obj_.fields_.push_back (f);
							return true;
						}
						else if (itr->find ("list") != std::string::npos )
						{
							field_t f;
							f.name_ = name;
							f.required_ = required_;
							f.type_ = field_t::e_list;
							current_obj_.fields_.push_back (f);
							return true;;
						}
						else if(itr->find("vector") != std::string::npos )
						{
							field_t f;
							f.name_ = name;
							f.required_ = required_;
							f.type_ = field_t::e_vector;
							current_obj_.fields_.push_back(f);
							return true;;
						}
						else if(itr->find("map") != std::string::npos)
						{
							field_t f;
							f.name_ = name;
							f.type_ = field_t::e_map;
							current_obj_.fields_.push_back(f);
							return true;;
						}
					}
				}
				else if (first.find("acl")!= std::string::npos)
				{
					for(std::list<std::string>::iterator itr = tokens.begin();
						itr != tokens.end(); ++itr)
					{
						if(itr->find("string") != std::string::npos)
						{
							field_t f;
							f.name_ = name;
							f.required_ = required_;
							f.type_ = field_t::e_string;
							current_obj_.fields_.push_back(f);
							return true;
						}
					}
					throw syntax_error();
				}
				else if (first == "singned" ||
						  first == "int" ||
						  first == "int" ||
						  first == "uint32_t" ||
						  first == "int32_t" ||
						  first == "int64_t" ||
						  first == "int64_t")
				{
					field_t f;
					f.type_ = field_t::e_number;
					f.name_ = name;
					f.required_ = required_;
					current_obj_.fields_.push_back (f);
					return true;
				}
				else if (first == "bool")
				{
					field_t f;
					f.type_ = field_t::e_bool;
					f.name_ = name;
					f.required_ = required_;
					current_obj_.fields_.push_back (f);
					return true;

				}
				else if (first == "float" ||
						 first == "double")
				{
					field_t f;
					f.type_ = field_t::e_double;
					f.name_ = name;
					f.required_ = required_;
					current_obj_.fields_.push_back (f);
					return true;
				}
				else
				{
					// user define class ,struct.
					field_t f;
					f.name_ = name;
					f.required_ = required_;
					f.type_ = field_t::e_object;
					current_obj_.fields_.push_back (f);
					return true;
				}
				return true;

			}
			return false;
		}
	
		bool read_file (const char *filepath)
		{
			std::ifstream is (filepath, std::ifstream::binary);
			if(!is)
				return false;
			std::string str ((std::istreambuf_iterator<char> (is)),
							 std::istreambuf_iterator<char> ());
			codes_ = str;

			filename_;
			int i = strlen(filepath) - 1;
			while(i >= 0 && (filepath[i] != '\\' || filepath[i] != '/'))
			{
				filename_.push_back(filepath[i]);
				i--;
			}
			std::reverse(filename_.begin(), filename_.end());

			return true;
		}
		void parse_code()
		{
			char c = '\n';
			max_pos_ = codes_.size ();
			try
			{
				do
				{
					try_skip_comment();
					if(pos_ == max_pos_)
						break;
					char ch = codes_[pos_];
					switch(ch)
					{
					case '/':
					{
						if(check_comment())
							continue;
					}
					case '}':
					{
						if(check_struct_end())
							continue;
						if(check_namespace_end())
							continue;
					}
					case 'n':
					{
						if(check_namespace())
							continue;
					}
					default:
					{
						if(check_struct_begin())
							continue;
						if(check_function())
							continue;
						if(check_member())
							continue;
						if(ch == '#' && check_include())
							continue;
						printf("%c", codes_[pos_]);
						pos_++;
					}
					}

				} while(pos_ < max_pos_);
			}
			catch(syntax_error &e)
			{
				printf(e.what());
				return;
			}
			
			catch (std::exception & e)
			{
				printf(e.what());
				return;
			}
			
			write_generate_file();
			std::cout << "OK" << std::endl;
			return ;
		}
		void write_generate_file()
		{
			const char *namespace_start = "namespace acl\n{\nnamespace gson\n{";
			const char *namespace_end = "\n}///end of acl.\n}///end of gson.";

			write_source("#include \"stdafx.h\"\n");
			write_source("#include \"" + filename_ + "\"\n");
			write_source("#include \"gson_helper.ipp\"\r\n");
			write_header(namespace_start);
			write_source(namespace_start);

			for(std::list<object_t>::iterator itr = objs_.begin();
				itr != objs_.end(); ++itr)
			{
				function_code_t pack = gen_pack_code(*itr);
				function_code_t unpack = gen_unpack_code(*itr);
				
				write_header(('\n' + tab_ + "//"+itr->name_));
				write_header(('\n' + tab_ + pack.declare2_));
				write_header(('\n'+tab_+ pack.declare_));
				write_header(('\n'+tab_+ pack.declare_ptr_));
				write_header('\n'+tab_+ unpack.declare_);

				write_source(add_4space(pack.definition_));
				write_source(add_4space(pack.definition_ptr_));
				write_source(add_4space(pack.definition2_));
				write_source(add_4space(unpack.definition_));
				write_source(add_4space(unpack.definition_ptr_));
			}
			write_header(namespace_end);
			write_source(namespace_end);
			flush();
		}
		std::string add_4space(const std::string &code)
		{
			std::string result;
			result += '\n';
			result += tab_;
			std::string tmp;
			int len = code.size();
			int i = 0;
			bool end = false;
			int syn = 0;
			while (i < len)
			{
				if(code[i] == '{')
					syn++;

				if(code[i] == '}')
				{
					syn--;
					if(syn == 0)
						end = true;
				}
				result.push_back(code[i]);

				if(end == false &&
				   code[i] == '\n'&&
				   code[i+1] != '\n'&& 
				   code[i + 1] != '\r' )
				{
					result += tab_;
				}
				i++;
			}
			return result;
		}
		void flush()
		{
			gen_header_->flush();
			gen_source_->flush();
			delete gen_header_;
			delete gen_source_;
		}
		void write_header(const std::string &data)
		{
			if(gen_header_ == NULL)
				gen_header_ = new std::ofstream("gson_gen.h");
			gen_header_->write(data.c_str(), data.size());
		}
		void write_source(const std::string &data)
		{
			if(gen_source_ == NULL)
				gen_source_ = new std::ofstream("gson_gen.cpp");
			gen_source_->write(data.c_str(), data.size());
		}
		void set_default_required()
		{
			default_ = true;
		}
		void set_default_optional()
		{
			default_ = false;
		}

	


		char cc;
		int pos_ = 0;
		int max_pos_;
		std::string comment_begin_;
		std::string comment_end_;
		std::string codes_;
		code_parser_status_t status_;
		std::string tab_ = "    ";
		bool required_;
		bool default_;
		object_t current_obj_;
		std::list<object_t> objs_;
		std::list<std::string> namespaces_;
		std::list<std::string> includes_;
		std::string  filename_;
		std::ofstream *gen_header_;
		std::ofstream *gen_source_;
	};

}//end of gson
}//end of acl
