#include "client.h"

int main(int argc, char **argv) {
	int control_sock = clientOpen(host, SERV_PORT);
	
	if(control_sock == -1)
	{
		exit(1);
	}
	
	char recvline[BUF_SIZE] = {0};
	if(recv(control_sock, recvline, BUF_SIZE, 0) < 0)
	{
		printf("recv connect message error!\n");
		exit(1);
	}
	
	printf("%s", recvline);
	#ifdef debug
	    printf("recv when connect successful in line 77.\n");
	#endif
	
	commandLoop(control_sock);
    
	return 0;
}

void getCommand(char* command)
{
	fgets(command, commdLen, stdin);
}

void transCommandToCmd(struct cmd* my_cmd, char* command)
{
	int i = 0, j = 0;
	int len = strlen(command);
	
	if(len == 0)
	{
		return;
	}
	
	int flag = 1; //0 cmd have para, 1 not
	
	while(command[j] == ' ')
	{
		j++;
	}

	while(j < len && command[j] != ' ' && command[j] != '\n')
	{
		my_cmd->command[i] = command[j];
		i++;
		j++;
	}
	
	my_cmd->command[i] = '\0';
	
	i = 0;
	j++;
	while(j < len && command[j] != '\n')
	{
		my_cmd->para[i] = command[j];
		flag = 0;
		i++;
		j++;
	} 
	
	my_cmd->para[i] = '\0';
	
	if(strcmp(my_cmd->command, "SYST") == 0 && flag == 1)
	{
		my_cmd->id = SYST;
	}
	else if(strcmp(my_cmd->command, "PASV") == 0 && flag == 1)
	{
		my_cmd->id = PASV;
	}
	else if(strcmp(my_cmd->command, "PORT") == 0 && flag == 0)
	{
		my_cmd->id = PORT;
	}
	else if(strcmp(my_cmd->command, "RETR") == 0 && flag == 0)
	{
		my_cmd->id = RETR;
	}
	else if(strcmp(my_cmd->command, "STOR") == 0 && flag == 0)
	{
		my_cmd->id = STOR;
	}
	else if(strcmp(my_cmd->command, "TYPE") == 0 && flag == 0)
	{
		my_cmd->id = TYPE;
	}
	else if(strcmp(my_cmd->command, "QUIT") == 0 && flag == 1)
	{
		my_cmd->id = QUIT;
	}
	else if(strcmp(my_cmd->command, "ABOR") == 0 && flag == 1)
	{
		my_cmd->id = ABOR;
	}
	else if(strcmp(my_cmd->command, "PWD") == 0 && flag == 1)
	{
		my_cmd->id = PWD;
	}
	else if(strcmp(my_cmd->command, "MKD") == 0 && flag == 0)
	{
		my_cmd->id = MKD;
	}
	else if(strcmp(my_cmd->command, "DELE") == 0 && flag == 0)
	{
		my_cmd->id = DELE;
	}
	else if(strcmp(my_cmd->command, "CWD") == 0 && flag == 0)
	{
		my_cmd->id = CWD;
	}
	else if(strcmp(my_cmd->command, "RMD") == 0 && flag == 0)
	{
		my_cmd->id = RMD;
	} 
	else if(strcmp(my_cmd->command, "RNFR") == 0 && flag == 0)
	{
		my_cmd->id = RNFR;
	}
	else if(strcmp(my_cmd->command, "RNTO") == 0 && flag == 0)
	{
		my_cmd->id = RNTO;
	}
	else if(strcmp(my_cmd->command, "CDUP") == 0 && flag == 1)
	{
		my_cmd->id = CDUP;
	}
	else if(strcmp(my_cmd->command, "help") == 0 && flag == 1)
	{
		my_cmd->id = help;
	}
	else
	{
		my_cmd->id = 0;
	}
}

int dealCommand(int sockfd, char* command)
{
	#ifdef debug
	    printf("get command %s\n", command);
	#endif
	struct cmd *my_cmd = (struct cmd*)malloc(sizeof(struct cmd));
	memset(my_cmd, 0, sizeof(struct cmd));
	transCommandToCmd(my_cmd, command);
	#ifdef debug
	    printf("after deal my_cmd->id %d\n", my_cmd->id);
	#endif

	switch(my_cmd->id)
	{
		case SYST:
		    command_syst(sockfd, my_cmd->command);
		    break;
	    case PASV:
		    command_pasv(sockfd, my_cmd->command);
		    break;
		case PORT:
		    command_port(sockfd, my_cmd);
		    break;
		case RETR:
		    command_retr(sockfd, my_cmd);
		    break;
		case STOR:
		    command_stor(sockfd, my_cmd);
		    break;
		case TYPE:
		    command_type(sockfd, my_cmd);
		    break;
		case QUIT:
		    command_quit(sockfd, my_cmd->command);
		    return -1;
		    break;
		case ABOR:
		    command_abor(sockfd, my_cmd->command);
		    return -1;
		    break;
		case PWD:
		    command_pwd(sockfd, my_cmd->command);
		    break;
		case MKD:
		    command_mkd(sockfd, my_cmd);
		    break;
		case DELE:
		    command_dele(sockfd, my_cmd);
		    break;
		case CWD:
		    command_cwd(sockfd, my_cmd);
		    break;
		case RMD:
		    command_rmd(sockfd, my_cmd);
		    break;
		case RNFR:
		    command_rnfr(sockfd, my_cmd);
		    break;
		case RNTO:
		    command_rnto(sockfd, my_cmd);
		    break;
		case CDUP:
		    command_cdup(sockfd, my_cmd->command);
		    break;
		case help:
		    command_help();
		    break;
		default:
		    printf("no such command. type 'help' for more information!\n");
		    break;
	}
	
	free(my_cmd); 
	return 0;
}

int findChar(char a, char* s, int len)
{
	int i = 0;
	for(i = 0; i < len; i++)
	{
		if(s[i] == a)
		{
			return 1;
		}
	}
	return -1;
}

char* itostr(char *str, int i) //将i转化位字符串存入str
{
    sprintf(str, "%d", i);
    return str;
} 

//login
int login(int sockfd)
{
	char sendline[BUF_SIZE] = {0};
	char recvline[BUF_SIZE] = {0};
	char command[commdLen] = {0};
	
	struct cmd* my_cmd = (struct cmd*)malloc(sizeof(struct cmd));
	memset(my_cmd, 0, sizeof(struct cmd));
	
	int equal = 0;
	do{
		printf(ID);
		getCommand(command);
		transCommandToCmd(my_cmd, command);
		
		#ifdef debug
		    printf("after deal my_cmd->command %s\n", my_cmd->command);
		#endif
		
		memset(command, 0, commdLen);
		strcpy(command, my_cmd->command);
		strcat(command, " ");
		strcat(command, my_cmd->para);
		int len = strlen(command);
		command[len] = '\0';
		if(strcmp(command, "USER anonymous") == 0)
		{
			equal = 1;
		}
		else
		{
			printf("please input 'USER anonymous'. \n");
		}
	}while(equal != 1);
	
	equal = 0;
	//send data
	strcpy(sendline, command);
	int len = strlen(sendline);
	sendline[len] = '\0';
	send(sockfd, sendline, BUF_SIZE, 0);
	#ifdef debug
	    printf("send cmd 'USER anonymous' in line 184.\n");
	#endif
	//recv data from server
	recv(sockfd, recvline, BUF_SIZE, 0);
	//print data
	printf("%s", recvline);
	#ifdef debug
	    printf("recv when send cmd 'USER anonymous' in line 191.\n"); 
	#endif
	
	//password
	if(strncmp(recvline, "331", 3) == 0)
	{
		while(1)
		{
			do{
				printf(ID);
				memset(command, 0, commdLen);
				getCommand(command);
				memset(my_cmd, 0, sizeof(struct cmd));
				transCommandToCmd(my_cmd, command);
				
				if(strcmp(my_cmd->command, "PASS") == 0 && findChar('@', my_cmd->para, strlen(my_cmd->para)) != -1)
				{
					equal = 1;
				}
				else
				{
					printf("please input PASS + your email.\n");	
				}
			}while(equal != 1);
			
			memset(sendline, 0, BUF_SIZE);
			strcpy(sendline, my_cmd->command);
			strcat(sendline, " ");
			strcat(sendline, my_cmd->para);
			len = strlen(sendline);
			sendline[len] = '\0';
			send(sockfd, sendline, BUF_SIZE, 0);
			#ifdef debug
			    printf("send cmd 'PASS' in line 224.\n");
			#endif
			memset(recvline, 0, BUF_SIZE);
			recv(sockfd, recvline, BUF_SIZE, 0);
			printf("%s", recvline);
			if(strncmp(recvline, "230", 3) == 0)
			{
				return 1;
			}
			#ifdef debug
			    printf("recv when send PASS cmd in line 234.\n");
			#endif
		}	
	}
	free(my_cmd);
	return 0;
}

int clientOpen(char* ip, int port)
{
	int sockfd;
	struct sockaddr_in addr;
    
    //create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
    
    //connect to server
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return -1;
	}
	
	return sockfd;
}

void commandLoop(int sockfd)
{
	//login first
	int login_yes = 0;
	char command[commdLen] = {0};
	while(1)
	{
		login_yes = login(sockfd);
		
		if(login_yes == 1)
		{
			//get command and deal
			while(1)
			{
				printf(ID);
				getCommand(command);
				//quit or abor
				if(dealCommand(sockfd, command) == -1)
				{
					goto exit;
				}
			}
		}
	}
exit:
	close(sockfd);
}

void command_syst(int sockfd, char* command)
{
	char recvline[BUF_SIZE] = {0};
	if(send(sockfd, command, commdLen, 0) < 0)
	{
		printf("error to send command %s\n", command);
		exit(1);
	}
	
	#ifdef debug
	    printf("send cmd 'SYST' in line 394. \n");
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
		exit(1); 
	}
	printf("%s", recvline);
	#ifdef debug
	    printf("recv after send cmd 'SYST'.\n");
	#endif
}

void command_pasv(int sockfd, char* command)
{	
	char recvline[BUF_SIZE] = {0};
	if(send(sockfd, command, commdLen, 0) < 0)
	{
		printf("error to send command %s\n", command);
		return;
	}
	
	#ifdef debug
	    printf("send cmd 'PASV' in line 418.\n"); 
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
		return; 
	}
	
	#ifdef debug
	    printf("recv after send cmd 'PASV' in line 429.\n"); 
	#endif
	
	if(strncmp(recvline,"227",3) == 0)
	{
		printf("%s",recvline);
		//already choose mode
		if(pasv_or_port != -1)
		{
			close(data_sock);
			pasv_or_port = -1;
		}
	}
	else
	{
		printf("pasv recv is error!\n");
		return;
	}    
	
	//处理ftp server 端口
	char *ptr1 = NULL,*ptr2 = NULL;
	char num[BUF_SIZE];
	int i = 0, j = 0;
	memset(num, 0, BUF_SIZE);
	//取低位字节
	ptr1= recvline + strlen(recvline);
	while(*(ptr1) != ')')
	{
		ptr1--;
	}
	ptr2 = ptr1;
	while(*(ptr2) != ',')
		ptr2--;
	strncpy(num, ptr2 + 1, ptr1 - ptr2 - 1);
	i = atoi(num);//将字符串转换成整数
	//取高位字节
	memset(num, 0, BUF_SIZE);
	ptr1 = ptr2;
	ptr2--;
	while(*(ptr2)!=',')
		ptr2--;
	strncpy(num,ptr2 + 1, ptr1 - ptr2 - 1);
	j = atoi(num);
	//初始化服务器数据连接时的端口信息
	int data_serviceport;
	data_serviceport = j * 256 + i;
	
	//get ip
    char myNum[4][20];
    memset(myNum[0], 0, 20);
    memset(myNum[1], 0, 20);
    memset(myNum[2], 0, 20);
    memset(myNum[3], 0, 20);
	char ip[50];
	memset(ip, 0, 50);
	ptr1 = ptr2;
	ptr2--;
	while(*(ptr2)!=',')
		ptr2--;
	strncpy(myNum[3],ptr2 + 1, ptr1 - ptr2 - 1);
	
	ptr1 = ptr2;
	ptr2--;
	while(*(ptr2)!=',')
		ptr2--;
	strncpy(myNum[2],ptr2 + 1, ptr1 - ptr2 - 1);
	
	ptr1 = ptr2;
	ptr2--;
	while(*(ptr2)!=',')
		ptr2--;
	strncpy(myNum[1],ptr2 + 1, ptr1 - ptr2 - 1);
	
	ptr1 = ptr2;
	ptr2--;
	while(*(ptr2)!='(')
		ptr2--;
	strncpy(myNum[0],ptr2 + 1, ptr1 - ptr2 - 1);
	
	strcpy(ip, myNum[0]);
	strcat(ip, ".");
	strcat(ip, myNum[1]);
	strcat(ip, ".");
	strcat(ip, myNum[2]); 
	strcat(ip, ".");
	strcat(ip, myNum[3]);
	ip[strlen(ip)] = '\0';
	
	#ifdef debug
	    printf("get ip: %s in line 562\n", ip);
	#endif
	
	data_sock = clientOpen(ip, data_serviceport);
	if(data_sock != -1)
	{
		pasv_or_port = 0;
	}
	else
	{
		printf("error to get data_sock\n");
	}
}

void command_port(int sockfd, struct cmd* my_cmd)
{	
	//get ip and port
	struct sockaddr_in addr;
	int v[6];
	sscanf(my_cmd->para, "%d,%d,%d,%d,%d,%d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
	
	int port = v[4] * 256 + v[5];
	if(port < 1025 || port > 65535)
	{
		printf("port cmd error. please input again.\n");
		return;
	}
	
	/*
	char ip[20];
	memset(ip, 0, 20);
	char str[10];
	itostr(str, v[0]);
	strcpy(ip, str);
	strcat(ip, ".");
	itostr(str, v[1]);
	strcat(ip, str);
	strcat(ip, ".");
	itostr(str, v[2]);
	strcat(ip, str);
	strcat(ip, ".");
	itostr(str, v[3]);
	strcat(ip, str);
	int len = strlen(ip);
	ip[len] = '\0';
	*/
	
	if((clientListenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);  //port
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //ip
	bind(clientListenfd, (struct sockaddr*)&addr, sizeof(addr));
	listen(clientListenfd, 10);
	
	//send cmd
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int lenStr = strlen(sendline);
	sendline[lenStr] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
		return;
	}
	
	#ifdef debug
	    printf("send cmd 'PORT' in line 502.\n"); 
	#endif
		
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
		return; 
	}
	
	#ifdef debug
	    printf("recv after send cmd 'PORT' in line 513.\n"); 
	#endif
	
	if(strncmp(recvline, "200", 3) == 0)
	{
		printf("%s", recvline);
		//already choose mode
		if(pasv_or_port != -1)
		{
			close(data_sock);
			pasv_or_port = -1;
		}
	}
	else
	{
		printf("port recv is error!\n");
		return;
	}
	
	pasv_or_port = 1;
}

void command_retr(int sockfd, struct cmd* my_cmd)
{
	//mode set
	if(pasv_or_port == -1)
	{
		printf("please choose a mode first.(pasv or port)\n");
		return;
	}
	
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	     printf("send cmd 'RETR' in line 556.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'RETR' in line 566. \n");
	#endif
	
	//pasv
	if(pasv_or_port == 0)
	{
		if(strncmp(recvline, "150", 3) == 0)
		{
			printf("%s", recvline);
		}
		else
		{
			printf("%s", recvline);
			return;
		}
	}
	else if(pasv_or_port == 1)
	{
		if(strncmp(recvline, "150", 3) == 0)
		{
			//get data_sock
			if((data_sock = accept(clientListenfd, NULL, NULL)) == -1)
			{
				printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				return;
			}
			printf("%s", recvline);
		}
		else
		{
			printf("%s", recvline);
			return;
		}
	}
	
	//transfer file
	int nread;
	char recvData[BUF_SIZE] = {0};
	//open file
	FILE *fp = fopen(my_cmd->para, "wb"); 
    if(fp == NULL){
        printf("fail to open file.\n");
    }
    
    else
    {
		for(; ;)
		{
			nread = recv(data_sock, recvData, BUF_SIZE, 0);
			if(nread < 0)
			{
				printf("receive data error!\n");
			}
			else if(nread == 0)
			{
				fclose(fp);
				break;
			}
			else
			{
				fwrite(recvData, nread, 1, fp);
			}
		}
	}
        
    if(close(data_sock) < 0)
    {
		printf("data_sock close error!\n");
	}
	else
	{
		pasv_or_port = -1;
	}
	
	#ifdef debug
        printf("recv after complete trans file in line 826. \n");
    #endif
    
	memset(recvline, 0, BUF_SIZE);
	
	struct timeval timeout = {1, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)& timeout, sizeof(struct timeval));

	recv(sockfd, recvline, BUF_SIZE, 0); 
    printf("%s", recvline);

    #ifdef debug
        printf("recv after complete trans file in line 834. \n");
    #endif
}

void command_stor(int sockfd, struct cmd* my_cmd)
{
	//mode set
	if(pasv_or_port == -1)
	{
		printf("please choose a mode first.(pasv or port)\n");
		return;
	}
	
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'STOR' in line 662. \n"); 
	#endif
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'STOR' in line 671. \n");
	#endif
	//pasv
	if(pasv_or_port == 0)
	{
		if(strncmp(recvline, "150", 3) == 0)
		{
			printf("%s", recvline);
		}
		else
		{
			printf("%s", recvline);
			return;
		}
		
	}
	else if(pasv_or_port == 1)
	{
		if(strncmp(recvline, "150", 3) == 0)
		{
			//get data_sock
			if((data_sock = accept(clientListenfd, NULL, NULL)) == -1)
			{
				printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				return;
			}
			
			printf("%s", recvline);
		}
		else
		{
			printf("%s", recvline);
			return;
		}
	}
	
	//transfer file
	//open file
	FILE *fp = fopen(my_cmd->para, "rb");
	int nread;
    if(fp == NULL){
        printf("fail to open file.\n");
    }
    
    else
    {
		char sendData[BUF_SIZE] = {0};
		for(; ;)
		{
			nread = fread(sendData, 1, BUF_SIZE, fp);
			if(nread > 0)
			{
				send(data_sock, sendData, nread, 0);
			}
			else
			{
				fclose(fp);
				break;
			}
		}
	}
    
    
    if(close(data_sock) < 0)
    {
		printf("data_sock close error!\n");
	}
	else
	{
		pasv_or_port = -1;
	}
	
	memset(recvline, 0, BUF_SIZE);
	recv(sockfd, recvline, BUF_SIZE, 0); 
    printf("%s", recvline);
    
    #ifdef debug
        printf("recv after complete trans file in line 730. \n");
    #endif 
}

void command_type(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'TYPE' in line 758.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'TYPE' in line 768\n");
	#endif
	
	printf("%s", recvline);
}

void command_quit(int sockfd, char* command)
{
	char sendline[BUF_SIZE] = {0};
	char recvline[BUF_SIZE] = {0};
	strcpy(sendline, command);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", command);
	}
	
	#ifdef debug
	    printf("send cmd 'QUIT' in line 791.\n");
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
	}
	printf("%s", recvline);
	#ifdef debug
	    printf("recv after send cmd 'QUIT' in line 800.\n");
	#endif	
	
	if(pasv_or_port != -1)
	{
		close(data_sock);
		pasv_or_port = -1;
	}
}

void command_abor(int sockfd, char* command)
{
	char sendline[BUF_SIZE] = {0};
	char recvline[BUF_SIZE] = {0};
	strcpy(sendline, command);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", command);
		exit(1);
	}
	
	#ifdef debug
	    printf("send cmd 'ABOR' in line 825.\n");
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
		exit(1); 
	}
	printf("%s", recvline);
	#ifdef debug
	    printf("recv after send cmd 'ABOR' in line 835.\n");
	#endif
	
	if(pasv_or_port != -1)
	{
		close(data_sock);
		pasv_or_port = -1;
	}
}

void command_pwd(int sockfd, char* command)
{
	char recvline[BUF_SIZE] = {0};
	if(send(sockfd, command, commdLen, 0) < 0)
	{
		printf("error to send command %s\n", command);
		exit(1);
	}
	
	#ifdef debug
	    printf("send cmd 'PWD' in line 394. \n");
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
		exit(1); 
	}
	printf("%s", recvline);
	#ifdef debug
	    printf("recv after send cmd 'PWD'.\n");
	#endif
}

void command_mkd(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'MKD' in line 758.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'MKD' in line 768\n");
	#endif
	
	printf("%s", recvline);
}

void command_dele(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'DELE' in line 1065.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'DELE' in line 1075\n");
	#endif
	
	printf("%s", recvline);
}

void command_cwd(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'CWD' in line 1105.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'CWD' in line 1115\n");
	#endif
	
	printf("%s", recvline);
}

void command_rmd(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'RMD' in line 1105.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'RMD' in line 1115\n");
	#endif
	
	printf("%s", recvline);
}

void command_rnfr(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'FNFR' in line 1105.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'FNFR' in line 1115\n");
	#endif
	
	printf("%s", recvline);
}

void command_rnto(int sockfd, struct cmd* my_cmd)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, my_cmd->command);
	strcat(sendline, " ");
	strcat(sendline, my_cmd->para);
	int len = strlen(sendline);
	sendline[len] = '\0';
	
	if(send(sockfd, sendline, BUF_SIZE, 0) < 0)
	{
		printf("error to send command %s\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("send cmd 'FNTO' in line 1105.\n");
	#endif
	
	char recvline[BUF_SIZE] = {0};
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", my_cmd->command);
	}
	
	#ifdef debug
	    printf("recv after send cmd 'FNTO' in line 1115\n");
	#endif
	
	printf("%s", recvline);
}

void command_cdup(int sockfd, char* command)
{
	char recvline[BUF_SIZE] = {0};
	if(send(sockfd, command, commdLen, 0) < 0)
	{
		printf("error to send command %s\n", command);
		exit(1);
	}
	
	#ifdef debug
	    printf("send cmd 'CUDP' in line 394. \n");
	#endif
	
	if(recv(sockfd, recvline, BUF_SIZE, 0) < 0)
	{
		printf("error to recv command %s\'s response\n", command);
		exit(1); 
	}
	printf("%s", recvline);
	#ifdef debug
	    printf("recv after send cmd 'CUDP'.\n");
	#endif
}

void command_help()
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, "all commands allowed are as follows: \n");
	strcat(sendline, "1. SYST\n");
	strcat(sendline, "2. TYPE I\n");
	strcat(sendline, "3. PASV\n");
	strcat(sendline, "4. PORT + your ip + port like PORT 127,0,0,1,123,34\n");
	strcat(sendline, "5. RETR + fileName you want like RETR 1.txt\n");
	strcat(sendline, "6. STOR + filename you want like STOR 2.txt\n");
	strcat(sendline, "7. QUIT\n");
	strcat(sendline, "8. ABOR\n");
	strcat(sendline, "9. PWD\n");
	strcat(sendline, "10. CWD + next directory\n");
	strcat(sendline, "11. MKD + directory\n");
	strcat(sendline, "12. DELE + file\n");
	strcat(sendline, "13. RMD + dirctory\n");
	strcat(sendline, "14. RNFR + old path\n");
	strcat(sendline, "15. RNTO + new path\n");
	strcat(sendline, "16. CDUP\n");
	strcat(sendline, "17. help\n");
	printf("%s\n", sendline);	
}
