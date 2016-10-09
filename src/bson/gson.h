#pragma once
#include "bson.h"
#include "struct.h"

using result_t = std::pair<bool, std::string>;

bool gson(bson_iter_t &iter, user_t &obj);
bool gson(bson_iter_t &iter, user_t *obj); 
bool gson(bson_t &bson, user_t &obj);
bool gson(bson_t &bson, group_t &obj); 
bool gson(bson_iter_t &iter, group_t &obj);

bool gson(const user_t &user, bson_t &bson, result_t *result = nullptr);
bool gson(const user_t *user, bson_t &bson, result_t *result = nullptr);
bool gson(const group_t &group, bson_t &bson, result_t *result = nullptr);
bool gson(const group_t *group, bson_t &bson, result_t *result = nullptr);