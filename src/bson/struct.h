#pragma once
#include <string>
#include <list>
#include "bson-types.h"
#include <vector>
#include <map>

struct user_t
{
	int id;
	std::string *username;
	~user_t()
	{
		if(username)
			delete username;
	}
	user_t(const user_t& user)
	{
		username = new std::string(user.username->c_str());
		id = user.id;
	}
	user_t ()
	{
		username = NULL;
	}
	user_t (int, std::string &&username)
		:id (int{((0))})
	{

	}
};

struct group_t
{
	~group_t()
	{
		if(double_ptr_)
			delete double_ptr_;
	}
	double double_;
	double *double_ptr_;
	std::string group_id;
	bson_oid_t obj_id_;
	user_t user;
	std::list<user_t> list_users;
	std::vector<user_t> vector_users;
	std::map<std::string, user_t>  map_users;
	std::map<std::string, std::list<user_t>>  map_list_users;
	std::list<std::map<std::string, user_t>> list_map_users;
};