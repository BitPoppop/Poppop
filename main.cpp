<<<<<<< HEAD
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cerrno>
=======
#include "Service.h"
#include "DTO.h"
//#include"SplitStr.h"
#include "DBContextFactory.h"
#include "string"
#include <sqlpp11/mysql/mysql.h>
#include <sqlpp11/sqlpp11.h>

#include <cassert>
#include <iostream>
>>>>>>> 68bbc0f23fa63be97151771eb03278d696dc8c46
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "DTO.h"
#include "Service.h"

using namespace std;
#define SERVER_PORT 8088 // 端口
#define MAX_LISTEN 5     // 最大监听数
#define MAX_BUF 1024     // 缓冲区大小

int usercount = 0;

struct pthread_data
{
    struct sockaddr_in client_addr;
    int my_fd;
};

void ErrorHandling(char *message); // 错误处理函数
void *ServerForClient(void *arg);

class CMD
{
public:
    char name[10];
    bool (*fun)(int);
};

CMD cmdlist[] =
    {
        {"login", Login},   //用户登录
        {"send", Send},     //发送消息
        {"create", Create}, //创建群聊
        {"create", CreateUser},
        // {"sendfile", Sendfile}, //发送文件
        // {"getfile", Getfile}, //获取文件
        // {"list"， List}, //获取好友列表（已排序）
        // {"show", Show}, //聊天记录
        // {"help", Help}, //获取帮助
        // {"exit", Exit}, //退出页面
        {"quit", Quit}, //下线指令
        // {"find", Find}, //列表中查找好友
        // {"search", Search}, //网络中查找好友
        {"apply", Apply}, //申请好友
        {"reply", Reply}, //验证好友
        // {"clear", Clear}, //清空页面
        // {"remove", Remove}, //撤回消息
        {"hello", Hello}};

//解析并处理命令
int ExecCmd(int client_fd, char *cmd)
{
    char *argv[10] = {NULL};
    int argc = 0;
    int i = 0;

    if (strlen(cmd) == 0)
        return 0;

    char msg[MAX_BUF + 1];
    strcpy(msg, cmd);

    /*以字符' '对命令进行切割 */
    while ((argv[argc] = strtok((argc == 0 ? cmd : NULL), " ")) != NULL)
    {
        printf("argv[%d] = %s\n", argc, argv[argc]);
        argc++;
    }

    /* 查找命令 */
    for (i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++)
    {
        if (strcmp(cmdlist[i].name, argv[0]) == 0)
        {
            /* 执行命令*/
            cmdlist[i].fun(client_fd);
            return 1;
        }
    }
    printf("cmd not find\n");
    return 0;
}

// 主函数
int main(int argc, char *argv[])
{
    int sockfd, new_fd;
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in sever_addr, client_addr;
    struct pthread_data pdata;

    bzero(&sever_addr, sizeof(sever_addr));
    sever_addr.sin_family = AF_INET;
    sever_addr.sin_port = htons(SERVER_PORT);
    sever_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 创建套接字，采用TCP协议
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        ErrorHandling((char *)"socket error");

    // 绑定
    if (bind(sockfd, (struct sockaddr *)&sever_addr, sizeof(struct sockaddr)) == -1)
        ErrorHandling((char *)"bind error");

    // 监听
    if (listen(sockfd, MAX_LISTEN) == -1)
        ErrorHandling((char *)"listen error");

    // 服务器开启服务
    puts("service open");
    while (1)
    {
        // 接受连接请求
        if ((new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) == -1)
        {
            perror("accept error!");
            continue;
        }
        else
            puts("new client connected...");

        // 创建新线程
        pthread_t *pt = (pthread_t *)malloc(sizeof(pthread_t));
        pdata.client_addr = client_addr;
        pdata.my_fd = new_fd;
        pthread_create(pt, NULL, ServerForClient, (void *)&pdata);
    }

    // 关闭服务器
    close(new_fd);
    close(sockfd);
    return 0;
}

/**************************************************/
/*名称：int Login(int client_fd)
 /*描述：用户登录
 /*作成日期： 2019-8-30
 /*参数：
 参数1：用户线程号、int、输入
 /*返回值：执行状态、int、1成功 0失败
 /*作者：kk
 /***************************************************/
bool Login(int client_fd)
{
    LoginDTO ret;
    char buf[MAX_BUF + 1];
    char *argv[10] = {NULL};
    int argc = 0;
    socklen_t len;
    len = recv(client_fd, buf, MAX_BUF, 0);

    while ((argv[argc] = strtok((argc == 0 ? buf : NULL), " ")) != NULL)
    {
        argc++;
    }
    ret.ID = atoi(argv[1]);
    ret.PassWord = argv[2];
    UserDTO get_from_DB;
    get_from_DB = SelectedByID(ret.ID);
    if (get_from_DB.Online_State == 1)
    {
        printf("Already online\n");
    }
    if (ret.PassWord.compare(get_from_DB.PassWord) == 0)
    {
        send(client_fd, "1", 2 * sizeof(char), 0);
        UserService::PutUserOnline(ret.ID, client_fd);
        vector<MessageDTO> vec;
        AskForMsgUnseen(client_fd, vec);
    }
    else
    {
        send(client_fd, "0", 2 * sizeof(char), 0);
    }
    return true;
}

/**************************************************/
/*名称：bool Quit(int client_fd)
 /*描述：用户下线
 /*作成日期： 2019-8-30
 /*参数：
     参数1：用户线程号、int、输入
 /*返回值：执行状态、int、-1退出
 /*作者：kk
 /***************************************************/
bool Quit(int client_fd)
{
    while (PutUserOffline(client_fd) == 0)
        ;
    return -1;
}

/**************************************************/
/*名称：bool Apply(int client_fd)
/*描述：添加好友
/*作成日期： 2019-8-30
/*参数：
    参数1：用户线程号、int、输入
/*返回值：执行状态、int、1成功
/*作者：kk
/***************************************************/
bool Apply(int client_fd)
{
    MessagePrivateDTO ret;
    char buf[MAX_BUF + 1];
    char *argv[10] = {NULL};
    int argc = 0;
    socklen_t len;
    len = recv(client_fd, buf, MAX_BUF, 0);

    while ((argv[argc] = strtok((argc == 0 ? buf : NULL), " ")) != NULL)
    {
        argc++;
    }
    ret.Type = -1; //额外消息类型
    ret.Sender_ID = atoi(argv[0]);
    ret.Recver_ID = atoi(argv[1]);
    ret.Context = argv[2];
    ret.Time = "";
    vector<FriendDTO> get_from_DB;
    GetFriendList(get_from_DB, ret.Sender_ID);
    // for(int i = 0; i < get_from_DB.size(); i++)
    // {
    //     if(get_from_DB[i].UserTwoID == ret.RecverID)
    //         return 0;
    // }
    UserDTO userThis;
    UserDTO userThat;
    userThis = UserService::SelectedByID(ret.Sender_ID);
    userThat = UserService::SelectedByID(ret.Recver_ID);
    string rbuf = "";
    string str1 = "apply";
    string str2 = argv[0];
    rbuf = str1 + " " + str2 + " " + ret.Context;
    //
    char *str = const_cast<char *>(rbuf.c_str());
    if (userThat.Online_State == 1)
    {
        printf("online\n");
        send(userThat.IP_Addr, str, sizeof(str) * sizeof(char), 0);
    }
    else
    {
        MessageDTO msg;
        msg.Context = rbuf;
        msg.Time = "";
        msg.Sender_ID = UserThis.ID;
        msg.Recver_ID = UserThat.ID;
        MessageService::Add(msg);
        printf("offline\n");
        send(UserThat.IP_Addr, str, sizeof(str) * sizeof(char), 0);
    }
}

/**************************************************/
/*名称：bool Reply(int client_fd)
/*描述：添加好友
/*作成日期： 2019-8-30
/*参数：
    参数1：用户线程号、int、输入
/*返回值：执行状态、int、1成功
/*作者：kk
/***************************************************/
/***************************************************/
/*与数据库的对接
/*wlj
/*2019-9-2
/**************************************************/
bool Reply(int client_fd)
{
    char buf[MAX_BUF + 1];
    char *argv[10] = {NULL};
    int argc = 0;
    socklen_t len;
    len = recv(client_fd, buf, MAX_BUF, 0);
    while ((argv[argc] = strtok((argc == 0 ? buf : NULL), " ")) != NULL)
    {
        argc++;
    }
    MessageDTO ret;
    ret.Sender_ID = atoi(argv[0]);
    ret.Recver_ID = atoi(argv[1]);
    UserDTO userThis;
    UserDTO userThat;
    userThis = UserService::SelectedByID(ret.Sender_ID);
    userThat = UserService::SelectedByID(ret.Recver_ID);
    if (argv[2] == 0)
    {
        ret.Context = "对不起，你是个好人,但我连朋友也不想和你做！";
    }
    else
    {
        ret.Context = "我们已经是好友了，一起来聊天吧！";
        FriendDTO friendDTO;
        friendDTO.This_ID;
        friendDTO.That_ID;
        FriendService::Add(friendDTO);
    }
    ret.Time = "";
    printf("To User : %d %d\n", ret.Recver_ID, userThat.IP_Addr);
    string rbuf = "";
    string str1 = "reply";
    string str2 = argv[0];
    rbuf = str1 + str2 + ret.Context;
    if (userThat.Online_State == 1)
    {
        send(userThat.IP_Addr, rbuf.c_str(), 2 * sizeof(rbuf.c_str()), 0);
    }
    else
    {
        send(userThat.IP_Addr, str1.c_str(), 2 * sizeof(str1.c_str()), 0);
        MessageDTO msg;
        msg.Context = rbuf;
        msg.Time = "";
        msg.Sender_ID = userThis.ID;
        msg.Recver_ID = userThat.ID;
        MessageService::Add(msg);
    }
}

/**************************************************/
/*名称：bool CreateUser(int client_fd)
/描述：用户注册
/*2019-9-2
/wlj
/**************************************************/
bool CreateUser(int client_fd)
{
    char buf[MAX_BUF + 1];
    char *argv[10] = {NULL};
    int argc = 0;
    socklen_t len;
    len = recv(client_fd, buf, MAX_BUF, 0);
    while ((argv[argc] = strtok((argc == 0 ? buf : NULL), " ")) != NULL)
    {
        argc++;
    }
    UserDTO userDTO;
    userDTO.Name = argv[0];
    userDTO.Sex = argv[1];
    userDTO.PassWord = argv[2];
    userDTO.Msg_to_recv = 0;
    userDTO.Department_Name = argv[3];
    userDTO.Motto = argv[4];
    userDTO.IP_Addr = client_fd;
    UserService::Add(userDTO);
}

// 错误处理函数
void ErrorHandling(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void *ServerForClient(void *arg)
{
    struct pthread_data *pdata = (struct pthread_data *)arg;
    int client_fd = pdata->my_fd;
    socklen_t len;
    char buf[MAX_BUF + 1];
    while (1)
    {
        bzero(buf, MAX_BUF + 1);
        len = recv(client_fd, buf, MAX_BUF, 0);
        if (len > 0)
        {
            if (ExecCmd(client_fd, buf) == -1)
                break;
            else if (ExecCmd(client_fd, buf) == 0)
                printf("ERROR\n");
            else
                printf("OK1\n");
        }
    }
}
