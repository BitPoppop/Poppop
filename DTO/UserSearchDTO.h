#ifndef USERSEARCHDTO_H
#define USERSEARCHDTO_H
#include"SearchDTO.h"
#include<string>
using namespace std;
class UserSearchDTO:public SearchDTO{      //查找用户
public:
	string Sex;     //所查询用户的性别
	string Department_Name;  //所查询用户的部门
};

#endif
