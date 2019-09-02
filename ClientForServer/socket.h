#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>

extern const int MAX_LISTEN; // 最大监听数
extern const int SERVER_PORT; // 端口
extern const char SERVER_IPV4[]; // ip地址
extern const int MAX_BUF; // 缓冲区大小
extern int fd; // socked通道

// 建立连接
extern void Connect();

// 注册，参数为用户名与密码，返回值为用户id
extern int Register(const char* usrname, const char* password);

// 登陆,参数为用户id与密码，返回值为0登陆失败，1为登陆成功
extern int Login(int id, const char* password);

// 发送信息函数，返回值0为失败，1为成功
extern int SendMessege(const char* msg);

// 发送文件，参数为文件所在目录，返回值0为失败，1为成功
extern int SendFile(const char* catalog);

// 接收消息，返回所接受消息
extern char* const RecvMessege();

// 接收文件，参数为文件保存目录,格式为"/../../"，返回值0为失败，1为成功
extern int RecvFile(const char* catalog);

// 建群，参数为所有用户的数组，返回值为所建群号
extern int CreateGroup(int* usr_id);

// 申请好友，参数为为我的id和被申请好友id,返回值0为失败，1为成功
extern int ApplyForFriend(int my_id, int fri_id);

// 同意好友与否
extern int ReplyFriend();

// 关闭连接
extern void Close();

#endif