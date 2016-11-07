#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <netdb.h>

#include <time.h>
#include <sys/types.h>

#define BUF_SIZE 1024
#define commdLen 200

//#define debug 1 //for debug  
char work_root[1000] = {0};

enum COMMAND
	{
	    USER = 1,
	    PASS,
	    SYST,
	    PASV,
	    PORT,
	    RETR,
	    STOR,
	    TYPE,
	    QUIT,
	    ABOR,
	    PWD,
	    MKD,
	    CWD,
	    DELE,
	    RMD,
	    RNFR,
	    RNTO,
	    CDUP
	};
	
struct cmd
{
	int id;
	char commd[commdLen];
	char para[commdLen];
};

struct oneClient
{
	int data_sock;
	int d_sock;   //for file trans
	
	int pasv_or_port; //mode
	int p_port;  //port mode port
	
	int writeMsg; //0 not write yet, 1 write already
	char message[BUF_SIZE * 2];  //for log
	
	char currPath[BUF_SIZE]; //current path
	
	char oldPath[200]; //for rnfr
	char newPath[200]; //for rnto
};  
int findChar(char a, char* s, int len); //find a char from char* s, return -1 if not find
char* itostr(char *str, int i); //将i转化位字符串存入str

void getServerPort(int* server_port, int argc, char** argv);
void getServerRoot(char* work_root, int len, int argc, char** argv);
int backToFatherDic(char* path); //for cudp cmd

void transCommandToCmd(struct cmd* my_cmd, char* my_command);
int dealCommand(int connfd, char* my_command, struct oneClient* oneCli);

void client_process(int connfd);   //do client connect cmd
void command_user(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_pass(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_syst(int connfd, struct oneClient* oneCli);
void command_pasv(int connfd, struct oneClient* oneCli);
void command_port(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_retr(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_stor(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_type(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_quit(int connfd, struct oneClient* oneCli);
void command_abor(int connfd, struct oneClient* oneCli);
void command_pwd(int connfd, struct oneClient* oneCli);
void command_mkd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_dele(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_cwd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_rmd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_rnfr(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_rnto(int connfd, struct cmd* my_cmd, struct oneClient* oneCli);
void command_cdup(int connfd, struct oneClient* oneCli);
void do_log(char* msg, struct oneClient* oneCli);
void log_out(int connfd, struct oneClient* oneCli);

#endif
