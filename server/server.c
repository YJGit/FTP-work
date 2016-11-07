#include "server.h"

int main(int argc, char **argv) {
	int listenfd, connfd;
	struct sockaddr_in addr;
	int server_port = 21;
	int client_pid;  //fork
    
    getServerPort(&server_port, argc, argv);
    getServerRoot(work_root, 1000, argc, argv);
    
    //create socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
    
    //bind socket to ip and port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server_port);  //port
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //ip

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
    
    //wait for request of client
	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	
    printf("FTP Server started up @ local:%d. Waiting for client(s)...\r\n", addr.sin_addr.s_addr);
		
	while (1) {
		//accept the request of client
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			printf("Error accept(): %s(%d)\n", strerror(errno), errno);
			break;
		}
		
		if((client_pid = fork()) > 0)
		{
			close(connfd);
			continue;
		} 
		else
		{
			if(client_pid == 0)
			{
				close(listenfd);
				client_process(connfd);
				exit(0);
			}
			else
			{
				printf("fork() error!");
				close(connfd);
				continue;
			}
		}
	}
	
	close(listenfd);	
    return 0;
}

void client_process(int connfd)
{
	char sendline[BUF_SIZE] = {0};
	
	struct oneClient *oneCli = (struct oneClient*)malloc(sizeof(struct oneClient));
	oneCli->data_sock = -1;
	oneCli->d_sock = -1;
	
	oneCli->pasv_or_port = -1;
	oneCli->p_port = -1;
	
	oneCli->writeMsg = 0; //0 not write yet, 1 write already
	memset(oneCli->message, 0, BUF_SIZE * 2);
	memset(oneCli->currPath, 0, BUF_SIZE);
	strcpy(oneCli->currPath, work_root);
	
	memset(oneCli->oldPath, 0, 200);
	memset(oneCli->newPath, 0, 200);
	
	//connect ok
	strcpy(sendline, "220 anonymous FTP server ready. \r\n");
	if(send(connfd, sendline, strlen(sendline), 0) < 0)
	{
		printf("Error send msg in line 107\n");
		return;
	}
	
	#ifdef debug
		printf("send connect ok in line 111.\n");
	#endif
	
	//get command and deal
	char commd[commdLen] = {0};
	while(1)
	{
		memset(commd, 0, commdLen);
		if(recv(connfd, commd, commdLen, 0) > 0)
		{
			#ifdef debug
				printf("get cmd %s\n", commd);
				printf("recv cmd in line 160.\n");
			#endif 
			if(dealCommand(connfd, commd, oneCli) == -1) //quit or abor
			{
				goto exit;
			}
		}
	}
exit:
	//close client socket
	close(connfd);
	free(oneCli);
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

//my_command may have a '\n' in end
void transCommandToCmd(struct cmd* my_cmd, char* my_command)
{
	int i = 0, j = 0;
	int len = strlen(my_command);
	int flag = 1; //0 cmd have para, 1 not
	
	while(my_command[j] == ' ')
	{
		j++;
	}

	while(j < len && my_command[j] != ' ' && my_command[j] != '\n')
	{
		my_cmd->commd[i] = my_command[j];
		i++;
		j++;
	}
	
	my_cmd->commd[i] = '\0';
	
	i = 0;
	j++;
	while(j < len && my_command[j] != '\n')
	{
		my_cmd->para[i] = my_command[j];
		flag = 0;
		i++;
		j++;
	}
	my_cmd->para[i] = '\0';
	
	printf("after deal flag %d\n", flag);
	if(strcmp(my_cmd->commd, "USER") == 0 && flag == 0)
	{
		my_cmd->id = USER;
	}
	else if(strcmp(my_cmd->commd, "PASS") == 0 && flag == 0)
	{
		my_cmd->id = PASS;
	}
	else if(strncmp(my_cmd->commd, "SYST", 4) == 0 && flag == 1)
	{
		my_cmd->id = SYST;
	}
	else if(strncmp(my_cmd->commd, "PASV", 4) == 0 && flag == 1)
	{
		my_cmd->id = PASV;
	}
	else if(strcmp(my_cmd->commd, "PORT") == 0 && flag == 0)
	{
		my_cmd->id = PORT;
	}
	else if(strcmp(my_cmd->commd, "RETR") == 0 && flag == 0)
	{
		my_cmd->id = RETR;
	}
	else if(strcmp(my_cmd->commd, "STOR") == 0 && flag == 0)
	{
		my_cmd->id = STOR;
	}
	else if(strcmp(my_cmd->commd, "TYPE") == 0 && flag == 0)
	{
		my_cmd->id = TYPE;
	}
	else if(strncmp(my_cmd->commd, "QUIT", 4) == 0 && flag == 1)
	{
		my_cmd->id = QUIT;
	}
	else if(strncmp(my_cmd->commd, "ABOR", 4) == 0 && flag == 1)
	{
		my_cmd->id = ABOR;
	}
	else if(strcmp(my_cmd->commd, "PWD") == 0 && flag == 1)
	{
		my_cmd->id = PWD;
	}
	else if(strncmp(my_cmd->commd, "MKD", 3) == 0 && flag == 0)
	{
		my_cmd->id = MKD;
	}
	else if(strcmp(my_cmd->commd, "DELE") == 0 && flag == 0)
	{
		my_cmd->id = DELE;
	}
	else if(strcmp(my_cmd->commd, "CWD") == 0 && flag == 0)
	{
		my_cmd->id = CWD;
	}
	else if(strcmp(my_cmd->commd, "RMD") == 0 && flag == 0)
	{
		my_cmd->id = RMD;
	} 
	else if(strcmp(my_cmd->commd, "RNFR") == 0 && flag == 0)
	{
		my_cmd->id = RNFR;
	}
	else if(strcmp(my_cmd->commd, "RNTO") == 0 && flag == 0)
	{
		my_cmd->id = RNTO;
	}
	else if(strcmp(my_cmd->commd, "CDUP") == 0 && flag == 1)
	{
		my_cmd->id = CDUP;
	}
	else
	{
		my_cmd->id = 0;
	}
}

int dealCommand(int connfd, char* my_command, struct oneClient* oneCli)
{
	struct cmd *my_cmd = (struct cmd*)malloc(sizeof(struct cmd));
	memset(my_cmd, 0, sizeof(struct cmd));
	transCommandToCmd(my_cmd, my_command);
	#ifdef debug
		printf("after deal my_cmd->id %d\n", my_cmd->id);
		printf("after deal my_cmd->commd %s\n", my_cmd->commd);
		printf("after deal my_cmd->para %s\n", my_cmd->para);
	#endif
	switch(my_cmd->id)
	{
		case USER:
		    command_user(connfd, my_cmd, oneCli);
		    break;
		case PASS:
		    command_pass(connfd, my_cmd, oneCli);
		    break;
		case SYST:
		    command_syst(connfd, oneCli);
		    break;
		case PASV:
		    command_pasv(connfd, oneCli);
		    break;
		case PORT:
		    command_port(connfd, my_cmd, oneCli);
		    break;
		case RETR:
		    command_retr(connfd, my_cmd, oneCli);
		    break;
		case STOR:
		    command_stor(connfd, my_cmd, oneCli);
		    break;
		case TYPE:
		    command_type(connfd, my_cmd, oneCli);
		    #ifdef debug
				printf("after deal my_cmd->para %s\n", my_cmd->para);
			#endif
		    break;
		case QUIT:
		    command_quit(connfd, oneCli);
		    return -1;
		    break;
		case ABOR:
		    command_abor(connfd, oneCli);
		    return -1;
		    break;
		case PWD:
		    command_pwd(connfd, oneCli);
		    break;
		case MKD:
		    command_mkd(connfd, my_cmd, oneCli);
		    break;
		case DELE:
		    command_dele(connfd, my_cmd, oneCli);
		    break;
		case CWD:
		    command_cwd(connfd, my_cmd, oneCli);
		    break;
		case RMD:
		    command_rmd(connfd, my_cmd, oneCli);
		    break;
		case RNFR:
		    command_rnfr(connfd, my_cmd, oneCli);
		    break;
		case RNTO:
		    command_rnto(connfd, my_cmd, oneCli);
		    break;
		case CDUP:
		    command_cdup(connfd, oneCli);
		    break;
		default:
		    break;
	}
	
	free(my_cmd); 
	return 0;
}

char* itostr(char *str, int i) //将i转化位字符串存入str
{
    sprintf(str, "%d", i);
    return str;
} 

void getServerPort(int* server_port, int argc, char** argv)
{
	int i;
    for (i = 0; i < argc; i ++) {
        if (strcmp(argv[i], "-port") == 0) {
            *server_port = atoi(argv[i + 1]);
            break;
        }
    }
}

void getServerRoot(char* work_root, int len, int argc, char** argv)
{
	int i;
	strcpy(work_root, "/tmp");
    for (i = 0; i < argc; i ++) {
        if (strcmp(argv[i], "-root") == 0) {
			memset(work_root, 0, len);
            strcpy(work_root, argv[i + 1]);
            break;
        }
    }
    if (work_root[strlen(work_root) - 1] != '/')
        strcat(work_root, "/");
}

int backToFatherDic(char* path)
{
	char* ptr1 = path + strlen(path);
	char* ptr2 = path;
	while(*ptr1 != '/')
	{
		ptr1--;
	}
	ptr1--;
	while(*ptr1 != *ptr2 && *ptr1 != '/')
	{
		ptr1--;
	}
	
	if(ptr1 == ptr2)
	{
		return -1;
	}
	
	char buf[200] = {0};
	strncpy(buf, ptr2, ptr1 - ptr2 + 1);
	strcpy(path, buf); 
	return 0;
}

void command_user(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	//request for login
	if(strncmp(my_cmd->para, "anonymous", 9) == 0)
	{
		strcpy(sendline, "331 Guest login ok, please send your e-mail address as password.\r\n");
	}
	else 
	{
		strcpy(sendline, "501 error get user cmd, please set to anonymous.\r\n");
	}
	
	//strcpy(sendline, "331 Guest login ok, please send your e-mail address as password.\r\n");
	send(connfd, sendline, strlen(sendline), 0);
	#ifdef debug
	    printf("send user cmd response. %s\n", sendline);
	#endif
}

void command_pass(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	if(findChar('@', my_cmd->para, strlen(my_cmd->para)) != -1)
	{
		strcpy(sendline, "230 Guest login ok.\r\n");
	} 
	else
	{
		strcpy(sendline, "501 Guest login fail.\r\n");
	}
	
	send(connfd, sendline, strlen(sendline), 0);
	#ifdef debug
	    printf("send pass cmd response. %s\n", sendline);
	#endif
}

void command_syst(int connfd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	strcpy(sendline, "215 UNIX Type: L8\r\n");
	send(connfd, sendline, strlen(sendline), 0);
	#ifdef debug
	    printf("send cmd 'syst' response in line 320. %s\n", sendline);
	#endif
	do_log("you send a SYST cmd.\r\n", oneCli);
}

void command_pasv(int connfd, struct oneClient* oneCli)
{
	//if data_sock have already
	if(oneCli->pasv_or_port == 0)
	{
		close(oneCli->data_sock);
		oneCli->pasv_or_port = -1;
	}
	else if(oneCli->pasv_or_port == 1)
	{
		oneCli->pasv_or_port = -1;
	}
	
	char ip[30];
	struct sockaddr_in addr;
	//get host ip
	char hname[128];
    char temp[200];
    struct hostent *hent;
    int i;
    
    gethostname(hname, sizeof(hname));
    hent = gethostbyname(hname);

    for(i = 0; hent->h_addr_list[i]; i++) {
        sprintf(temp, "%s", inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
    }
    //printf("%s\n", temp);
    strcpy(ip, temp);
    
	//socklen_t len = sizeof(addr);
    //getsockname(connfd, (struct sockaddr *)&addr, &len);
    //ip = inet_ntoa(addr.sin_addr);

	int v[4];
	sscanf(ip, "%d.%d.%d.%d", &v[0], &v[1], &v[2], &v[3]);
    
	srand((unsigned)time(NULL));
	int port[2];
	//20000 ~ 65535
	int lp = rand()%(45536) + 20000;
	port[0] = lp / 256;
	port[1] = lp % 256;
	
	oneCli->data_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(lp);  //port
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //ip
	bind(oneCli->data_sock, (struct sockaddr*)&addr, sizeof(addr));
	listen(oneCli->data_sock, 10);
	
	char sendline[BUF_SIZE] = {0};
	sprintf(sendline, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d). \r\n", 
		v[0], v[1], v[2], v[3], port[0], port[1]);
	
	send(connfd, sendline, strlen(sendline), 0);
	#ifdef debug
	    printf("send cmd 'pasv' response in line 366.\n");
	#endif
	
	oneCli->pasv_or_port = 0;
	do_log("you send a PASV cmd.\r\n", oneCli);
}

void command_port(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{	
	//if data_sock have already
	if(oneCli->pasv_or_port == 0)
	{
		close(oneCli->data_sock);
		oneCli->pasv_or_port = -1;
	}
	else if(oneCli->pasv_or_port == 1)
	{
		oneCli->pasv_or_port = -1;
	}
	
	int v[6];
	char sendline[BUF_SIZE] = {0};

	sscanf(my_cmd->para, "%d,%d,%d,%d,%d,%d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]);
	oneCli->p_port = v[4] * 256 + v[5];
	if(oneCli->p_port <= 1024 || oneCli->p_port > 65535)
	{
		strcpy(sendline, "530 port is invalid. \r\n");
		send(connfd, sendline, strlen(sendline), 0);
		return; 
		
		#ifdef debug
	        printf("send cmd 'port' error response in line 396.\n");
	    #endif
	}
	
	char ip[20] = {0};
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
	
	#ifdef debug
	    printf("get ip: %s\n", ip);
	#endif
	
	struct sockaddr_in addr;
    
    //create socket
	if ((oneCli->d_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		memset(sendline, 0, BUF_SIZE);
		strcpy(sendline, "530 socket is invalid. \r\n");
		send(connfd, sendline, strlen(sendline), 0);
		return;
	}
    
    //connect to port
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(oneCli->p_port);
	//addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return;
	}
	
	if (connect(oneCli->d_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		memset(sendline, 0, BUF_SIZE);
		strcpy(sendline, "530 connect is invalid. \r\n");
		send(connfd, sendline, strlen(sendline), 0); 
		return;
	}
	
	memset(sendline, 0, BUF_SIZE);
	strcpy(sendline, "200 PORT command successful.\r\n");
	send(connfd, sendline, strlen(sendline), 0);
	
	#ifdef debug
	    printf("send cmd 'port' ok response in line 436.\n");
	#endif
	
	oneCli->pasv_or_port = 1;
	
    do_log("you send a PORT cmd.\r\n", oneCli);
}

void command_retr(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	
	//choose mode first
	if(oneCli->pasv_or_port == -1)
	{
		strcpy(sendline, "530 please set mode first. \r\n");
		send(connfd, sendline, strlen(sendline), 0);
		#ifdef debug
	        printf("send set mode response in line 636.\n");
	    #endif
		return;
	}
	
	char filePath[200];
	memset(filePath, 0, 200);
	
    #ifdef debug
        printf("current path %s\n", oneCli->currPath);
    #endif
    
    strcpy(filePath, oneCli->currPath);
	strcat(filePath, my_cmd->para);
	
	int len = strlen(filePath);
	#ifdef debug
	    printf("%d\n", filePath[len-1]);
	    printf("filePath %s\n", filePath);
	#endif
	
	if(filePath[len - 1] == '\n' || filePath[len - 1] == '\r')
	{
		filePath[len - 1] = '\0';
	}
	
	#ifdef debug
	    printf("%d\n", filePath[len-1]);
	    printf("filePath1 %s\n", filePath);
	#endif
	
	
	FILE* fp = fopen(filePath, "rb");  
	memset(sendline, 0, BUF_SIZE);
	int nread;
	int fileSize = 0;
    if(fp == NULL){
        printf("fail to open file\n");
        
        //if data_sock have already
		if(oneCli->pasv_or_port == 0)
		{
			close(oneCli->data_sock);
			oneCli->pasv_or_port = -1;
		}
		else if(oneCli->pasv_or_port == 1)
		{
			oneCli->pasv_or_port = -1;
		}
	
        strcpy(sendline, "502 file open error.\r\n");
	    send(connfd, sendline, strlen(sendline), 0);
	    #ifdef debug
	        printf("send cmd 'retr' file open fail response in line 475.\n");
	    #endif
	    return;
    }
    
    else
    {    
		memset(sendline, 0, BUF_SIZE);
		//pasv mode
		if(oneCli->pasv_or_port == 0)
		{			
			//get d_sock
			if((oneCli->d_sock = accept(oneCli->data_sock, NULL, NULL)) == -1)
			{
				printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				//send error 
				strcpy(sendline, "520 accept error.\r\n");
				send(connfd, sendline, strlen(sendline), 0);
				return;
			}
	
			strcpy(sendline, "150 Opening BINARY mode data connection for ");	
		}
		//port mode
		else if(oneCli->pasv_or_port == 1)
		{
			strcpy(sendline, "150 Opening BINARY mode data connection for ");
		}
		strcat(sendline, my_cmd->para);
		strcat(sendline, "\r\n");
		send(connfd, sendline, strlen(sendline), 0);
		
		#ifdef debug
			printf("send cmd 'retr' response in line 605.\n");
		#endif
	
		char sendData[BUF_SIZE] = {0};
		for(; ;)
		{
			nread = fread(sendData, 1, BUF_SIZE, fp);
			fileSize += nread;
			if(nread > 0)
			{
				send(oneCli->d_sock, sendData, nread, 0);
			}
			else
			{
				fclose(fp);
				break;
			}
		}
	}
				
    if(close(oneCli->d_sock) < 0)
    {
		printf("d_sock close error!\n");
	}

	memset(sendline, 0, BUF_SIZE);
	strcpy(sendline, "226 transfer complete.\r\n");
	send(connfd, sendline, strlen(sendline), 0);
	
	#ifdef debug
	    printf("retr trans file complete response in line 808. \n");
	#endif
	
	memset(sendline, 0, BUF_SIZE);
	strcpy(sendline, "you send a RETR cmd.  ");
	strcat(sendline, "RETR for ");
	strcat(sendline, my_cmd->para);
	strcat(sendline, "(");
	char fileSizeStr[30] = {0};
	itostr(fileSizeStr, fileSize);
	strcat(sendline, fileSizeStr);
	strcat(sendline, " bytes)");
	strcat(sendline, "\r\n");
	do_log(sendline, oneCli);
}

void command_stor(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	
	//choose mode first
	if(oneCli->pasv_or_port == -1)
	{
		strcpy(sendline, "530 please set mode first. \r\n");
		send(connfd, sendline, strlen(sendline), 0);
		#ifdef debug
	        printf("stor set mode response in line 528. \n");
	    #endif
		
		return;
	}
	
	int nread;
	int fileSize = 0;
	//open file
	char filePath[1000] = {0};
	
    strcpy(filePath, oneCli->currPath);
	strcat(filePath, my_cmd->para);
	#ifdef debug
	        printf("filePath %s\n", filePath);
	#endif
	
	int len = strlen(filePath);
	if(filePath[len - 1] == '\r' || filePath[len - 1] == '\n')
	{
		filePath[len - 1] = '\0';
	}
	
	#ifdef debug
	        printf("filePath1 %s\n", filePath);
	#endif
	FILE *fp = fopen(filePath, "wb");
    if(fp == NULL){
        printf("fail to open file.\n");
        
        //if data_sock have already
		if(oneCli->pasv_or_port == 0)
		{
			close(oneCli->data_sock);
			oneCli->pasv_or_port = -1;
		}
		else if(oneCli->pasv_or_port == 1)
		{
			oneCli->pasv_or_port = -1;
		}
	
        memset(sendline, 0, BUF_SIZE);
        strcpy(sendline, "520 file open error.\r\n");
	    send(connfd, sendline, strlen(sendline), 0);
	    #ifdef debug
	        printf("stor oen file  fail response in line 502. \n");
	    #endif
	    return;
    }
    
    else
    {
		memset(sendline, 0, BUF_SIZE);
		//pasv mode
		if(oneCli->pasv_or_port == 0)
		{	
			//get d_sock
			if((oneCli->d_sock = accept(oneCli->data_sock, NULL, NULL)) == -1)
			{
				printf("Error accept(): %s(%d)\n", strerror(errno), errno);
				//send error 
				strcpy(sendline, "520 accept error.\r\n");
				send(connfd, sendline, strlen(sendline), 0);
				return;
			}
	
			strcpy(sendline, "150 Opening BINARY mode data connection for ");	
		}
		//port mode
		else if(oneCli->pasv_or_port == 1)
		{
			strcpy(sendline, "150 Opening BINARY mode data connection for ");
		}
		
		strcat(sendline, my_cmd->para);
		strcat(sendline, "\r\n");
		send(connfd, sendline, strlen(sendline), 0);
		
		#ifdef debug
			printf("send cmd 'stor' ok response in line 554. \n");
		#endif
	
		char recvData[BUF_SIZE] = {0};
		for(; ;)
		{
			nread = recv(oneCli->d_sock, recvData, BUF_SIZE, 0);
			fileSize += nread;
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
	
	#ifdef debug
	    printf("stor file ok response in line 607. \n");
	#endif			
    if(close(oneCli->d_sock) < 0)
    {
		printf("d_sock close error!\n");
	}
	
	memset(sendline, 0, BUF_SIZE);
	strcpy(sendline, "226 transfer complete.\r\n"); 
	send(connfd, sendline, strlen(sendline), 0);  
	
	memset(sendline, 0, BUF_SIZE);
	strcpy(sendline, "you send a STOR cmd.  ");
	strcat(sendline, "STOR for ");
	strcat(sendline, my_cmd->para);
	strcat(sendline, "(");
	char fileSizeStr[30] = {0};
	itostr(fileSizeStr, fileSize);
 	strcat(sendline, fileSizeStr);
 	strcat(sendline, " bytes)");
	strcat(sendline, "\r\n");
	do_log(sendline, oneCli);
}

void command_type(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	if(strncmp(my_cmd->para, "I", 1) == 0)
	{
		strcpy(sendline, "200 Type set to I.\r\n");
		send(connfd, sendline, strlen(sendline), 0);
		
	    do_log("you send a type command.\n", oneCli);
	}
	else
	{
		strcpy(sendline, "520 please set to I. \r\n");
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	        printf("send cmd 'type' fail response in line 607. %s\n", sendline);
	#endif	
}

void command_quit(int connfd, struct oneClient* oneCli)
{
	strcat(oneCli->message, "221 ByeBye^_^.\n");
	log_out(connfd, oneCli);
	if(oneCli->pasv_or_port != -1)
	{
		close(oneCli->data_sock);
		oneCli->pasv_or_port = -1;
	}
}

void command_abor(int connfd, struct oneClient* oneCli)
{
	strcat(oneCli->message, "221 ByeBye^_^.\n");
	log_out(connfd, oneCli);
	if(oneCli->pasv_or_port != -1)
	{
		close(oneCli->data_sock);
		oneCli->pasv_or_port = -1;
	}
}

void command_pwd(int connfd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	sprintf(sendline, "257 '%s' is current directory.\r\n", oneCli->currPath);
	send(connfd, sendline, strlen(sendline), 0);
	#ifdef debug
	    printf("send pwd reply\n");
	#endif
	do_log("you send a pwd command.\n", oneCli);
}

void command_mkd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char path[100] = {0}, shellCmd[100] = {0};
	char sendline[BUF_SIZE] = {0};
	
	strcpy(path, oneCli->currPath);
	strcat(path, my_cmd->para);
	#ifdef debug
	    printf("mkd path %s\n", path);
	#endif
	sprintf(shellCmd, "mkdir %s", path);
	int suc = system(shellCmd);
	if(suc == 0){
		sprintf(sendline, "257 '%s' directory created.\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "550 '%s': file or directory already exists.\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	    printf("send mkd reply\n");
	#endif
	do_log("you send a mkd command.\n", oneCli);
}

void command_dele(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char path[100] = {0}, shellCmd[100] = {0};
	char sendline[BUF_SIZE] = {0};
	
	strcpy(path, oneCli->currPath);
	strcat(path, my_cmd->para);
	#ifdef debug
	    printf("dele path %s\n", path);
	#endif
	sprintf(shellCmd, "rm %s", path);
	int suc = system(shellCmd);
	if(suc == 0){
		sprintf(sendline, "250 '%s': file  is deleted.\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "550 '%s': not a file or can't be deleted or not found\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	    printf("send dele reply\n");
	#endif
	do_log("you send a dele command.\n", oneCli);
}

void command_cwd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	
	if(chdir(my_cmd->para) >= 0){
		getcwd(oneCli->currPath, BUF_SIZE);
		strcat(oneCli->currPath, "/");
		sprintf(sendline, "257 CWD command successful current dictionary is '%s'.\r\n", oneCli->currPath);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "550 '%s': No such file or directory.\r\n", my_cmd->para);
		send(connfd, sendline, strlen(sendline), 0);
	}
	
	#ifdef debug
	    printf("send cwd reply\n");
	#endif
	do_log("you send a cwd command.\n", oneCli);
}

void command_rmd(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char path[100] = {0}, shellCmd[100] = {0};
	char sendline[BUF_SIZE] = {0};
	
	strcpy(path, oneCli->currPath);
	strcat(path, my_cmd->para);
	#ifdef debug
	    printf("rm path %s\n", path);
	#endif
	sprintf(shellCmd, "rm -r %s", path);
	int suc = system(shellCmd);
	if(suc == 0){
		sprintf(sendline, "250 '%s': directory  is deleted.\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "550 '%s' can't be deleted or not found\r\n", path);
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	    printf("send rmd reply\n");
	#endif
	do_log("you send a rmd command.\n", oneCli);
}

void command_rnfr(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char shellCmd[100] = {0};
	char sendline[BUF_SIZE] = {0};
	
	memset(oneCli->oldPath, 0, 200);
	strcpy(oneCli->oldPath, oneCli->currPath);
	strcat(oneCli->oldPath, my_cmd->para);
	#ifdef debug
	    printf("rnfr path %s\n", oneCli->oldPath);
	#endif
	sprintf(shellCmd, "mv %s %s", oneCli->oldPath, oneCli->oldPath);
	int suc = system(shellCmd);
	if(suc == 0){
		sprintf(sendline, "350 '%s': File exits ready to rename.\r\n", oneCli->oldPath);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "550 '%s': no such file or dictionary.\r\n", oneCli->oldPath);
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	    printf("send rnfr reply\n");
	#endif
	do_log("you send a rnfr command.\n", oneCli);
}

void command_rnto(int connfd, struct cmd* my_cmd, struct oneClient* oneCli)
{
	char shellCmd[100] = {0};
	char sendline[BUF_SIZE] = {0};
	
	memset(oneCli->newPath, 0, 200);
	strcpy(oneCli->newPath, oneCli->currPath);
	strcat(oneCli->newPath, my_cmd->para);
	#ifdef debug
	    printf("rnto path %s\n", oneCli->newPath);
	#endif
	sprintf(shellCmd, "mv -r %s %s", oneCli->newPath, oneCli->newPath);
	int suc = system(shellCmd);
	if(suc == 0){
		sprintf(sendline, "250 '%s'renamed to  '%s' successfully.\r\n", oneCli->oldPath ,oneCli->newPath);
		send(connfd, sendline, strlen(sendline), 0);
	}
	else{
		sprintf(sendline, "501 '%s' can't be renamed.\r\n", oneCli->oldPath);
		send(connfd, sendline, strlen(sendline), 0);
	}
	#ifdef debug
	    printf("send rnto reply\n");
	#endif
	do_log("you send a rnto command.\n", oneCli);
}

void command_cdup(int connfd, struct oneClient* oneCli)
{
	char sendline[BUF_SIZE] = {0};
	if(backToFatherDic(oneCli->currPath) != -1)
	{
		strcpy(sendline, "250 back to Last level directory successfully.\r\n");
	}
	else
	{
		strcpy(sendline, "501 already the topest level directory.\r\n"); 
	}
	send(connfd, sendline, strlen(sendline), 0);
}

void do_log(char* msg, struct oneClient* oneCli)
{
	//able to write in
	int lenMsg = strlen(msg);
	int len = strlen(oneCli->message);
	if(lenMsg < BUF_SIZE * 2 - len)
	{
		if(oneCli->writeMsg == 0)
		{
			strcpy(oneCli->message, "221-");
			strcat(oneCli->message, msg);
			oneCli->writeMsg = 1;
		}
		else
		{
			strcat(oneCli->message, "221-");
			strcat(oneCli->message, msg);
		}
	}
}


void log_out(int connfd, struct oneClient* oneCli)
{
	send(connfd, oneCli->message, strlen(oneCli->message), 0);
	#ifdef debug
	    printf("send log in line 681. \n");
	#endif	
	do_log("you send a PWD cmd.\r\n", oneCli);
}

