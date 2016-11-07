#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define	ID "FTP-> "
#define SERV_PORT 21
#define commdLen 200

//#define debug 1 //for debug

char* host = "127.0.0.1";
int pasv_or_port = -1;// 0 for pasv 1 for port
int data_sock;  //for file trans
int clientListenfd; //for port listen in port cmd  

enum COMMAND
	{
	    SYST = 1,
	    PASV,
	    PORT,
	    RETR,
	    STOR,
	    TYPE,
	    QUIT,
	    ABOR,
	    PWD,
	    MKD,
	    DELE,
	    CWD,
	    RMD,
	    RNFR,
	    RNTO,
	    CDUP,
	    help
	};
	
struct cmd
{
	int id;
	char command[commdLen];
	char para[commdLen];
};
	
//function to deal command
void getCommand(char* command);
void transCommandToCmd(struct cmd* my_cmd, char* command);  //trans command to struct cmd
int dealCommand(int sockfd, char* command);

//extra function needed 
int findChar(char a, char* s, int len); //return -1 if not find
char* itostr(char *str, int i); //将i转化位字符串存入str

//ftp function
int clientOpen(char* ip, int port);
void commandLoop(int sockfd);
int login(int sockfd);  //login
void command_syst(int sockfd, char* command);
void command_pasv(int sockfd, char* command);
void command_port(int sockfd, struct cmd* my_cmd);
void command_retr(int sockfd, struct cmd* my_cmd);
void command_stor(int sockfd, struct cmd* my_cmd);
void command_type(int sockfd, struct cmd* my_cmd);
void command_quit(int sockfd, char* command);
void command_abor(int sockfd, char* command);
void command_pwd(int sockfd, char* command);
void command_mkd(int sockfd, struct cmd* my_cmd);
void command_dele(int sockfd, struct cmd* my_cmd);
void command_cwd(int sockfd, struct cmd* my_cmd);
void command_rmd(int sockfd, struct cmd* my_cmd);
void command_rnfr(int sockfd, struct cmd* my_cmd);
void command_rnto(int sockfd, struct cmd* my_cmd);
void command_cdup(int sockfd, char* command);
void command_help();   //help for client

#endif
