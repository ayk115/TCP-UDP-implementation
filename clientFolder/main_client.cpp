#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <ctype.h>
#include <fstream>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)

#define PATH_LEN 256
#define MD5_LEN 32

typedef struct file{
	char name[1024];
	char time[1024];
	int size;
	char type[200];
	char filemd5[MD5_LEN+1];
}fs;

//Global Variables for client side
char complete_command[1000];
char *command[100];
int command_count=0;
char global_time[100];
char server_send_data [1024];

void execute(int sock, struct sockaddr_in server_addr, char * type, socklen_t  * sock_size);
int parse(char *inputString, char *cmdArgv[]);
void scan_input()
{
	char c;
	int count=0,i;
	for(i=0;i<1000;i++)
		complete_command[i]='\0';
	printf(" $ ");
	//	scanf("%c",&c);
	while((c=getchar())!='\n'){
		strncat(complete_command,&c,1);
		count++;
		//		scanf("%c",&c);
	}
	complete_command[count++]='\0';
	command_count=parse(complete_command, command);
}
int parse(char *inputString, char *cmdArgv[])
{
	int cmdArgc = 0, terminate = 0;
	char *srcPtr = inputString;
	while(*srcPtr==' ' || *srcPtr=='\t')
		srcPtr++;

	//printf("parse fun%sends", inputString);
	while(*srcPtr != '\0' && terminate == 0)
	{
		*cmdArgv = srcPtr;
		cmdArgc++;
		//printf("parse fun2%sends", *cmdArgv);
		while(*srcPtr != ' ' && *srcPtr != '\t' && *srcPtr != '\0' && *srcPtr != '\n' && terminate == 0)
		{

			srcPtr++;
		}
		while((*srcPtr == ' ' || *srcPtr == '\t' || *srcPtr == '\n') && terminate == 0)
		{
			*srcPtr = '\0';
			srcPtr++;
		}
		cmdArgv++;
	}
	/*srcPtr++;
	 *srcPtr = '\0';
	 destPtr--;*/
	*cmdArgv = '\0';
	return cmdArgc;
}
char *concat(char c1[],char c2[]){
	char *ans = (char *)malloc(sizeof(char)*100);
	for(int i=0;i<strlen(c1);i++)ans[i]=c1[i];
	ans[strlen(c1)]=' ';
	for(int i=strlen(c1)+1;i<strlen(c1)+strlen(c2)+1;i++)ans[i]=c2[i-strlen(c1)-1];
	ans[strlen(c1)+strlen(c2)+1]='\0';
	return ans;
}
void parse_time(char time1[]){
	int i=0,j=0;
	char ans[100];
	char month[4];
	for(int i=4;i<7;i++)
		month[i-4]=time1[i];
	month[3]='\0';
	ans[4]='-';
	for(i=strlen(time1)-5;i<(strlen(time1)-1);i++)
		ans[j++]=time1[i];
	//printf("DEBUG month %s\n",month);
	if(strcmp(month,"Jan")==0){ans[j++]='0';ans[j]='1';}
	if(strcmp(month,"Feb")==0){ans[j++]='0';ans[j]='2';}
	if(strcmp(month,"Mar")==0){ans[j++]='0';ans[j]='3';}
	if(strcmp(month,"Apr")==0){ans[j++]='0';ans[j]='4';}
	if(strcmp(month,"May")==0){ans[j++]='0';ans[j]='5';}
	if(strcmp(month,"Jun")==0){ans[j++]='0';ans[j]='6';}
	if(strcmp(month,"Jul")==0){ans[j++]='0';ans[j]='7';}
	if(strcmp(month,"Aug")==0){ans[j++]='0';ans[j]='8';}
	if(strcmp(month,"Sep")==0){ans[j++]='0';ans[j]='9';}
	if(strcmp(month,"Oct")==0){ans[j++]='1';ans[j]='0';}
	if(strcmp(month,"Nov")==0){ans[j++]='1';ans[j]='1';}
	if(strcmp(month,"Dec")==0){ans[j++]='1';ans[j]='2';}

	if(time1[8]==' ')
		ans[++j]='0';
	else
		ans[++j]=time1[8];
	ans[++j]=time1[9];
	for(i=11;i<=18;i++)
		if(i!=13 && i!=16)
			ans[++j]=time1[11];
	ans[++j]='\0';
	for(i=0;i<strlen(ans);i++)
		global_time[i]=ans[i];
}

int CalcFileMD5(char *file_name, char *md5_sum)
{
#define MD5SUM_CMD_FMT "md5sum %." STR(PATH_LEN) "s 2>/dev/null"
	char cmd[PATH_LEN + sizeof (MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, file_name);
#undef MD5SUM_CMD_FMT
	printf("%s\n",file_name);	
	printf("%s\n",cmd);
	FILE *p = popen(cmd, "r");
	if (p == NULL) return 0;

	int i, ch;
	for (i = 0; i < MD5_LEN && isxdigit(ch = fgetc(p)); i++) {
		*md5_sum++ = ch;
	}

	*md5_sum = '\0';
	pclose(p);
	return i == MD5_LEN;
}

void IndexGet(int sock,char type[], int recv_data_int, struct sockaddr_in server_addr, socklen_t* sock_size, int bytes_recieved, char recv_data[],struct file * FileStructure)
{
	if(strcmp(type,"tcp")==0)
	{
		send(sock,complete_command,strlen(complete_command),0);
		recv(sock,&recv_data_int,sizeof(recv_data_int),0);
	}
	else
	{
		sendto(sock,complete_command,strlen(complete_command),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
		recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,sock_size);
	}
	int file_count = recv_data_int;
	for(int i=0;i<file_count;i++)
	{
		if(strcmp(type,"tcp")==0)
			bytes_recieved = recv(sock,recv_data,1024,0);
		else
			bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
		recv_data[bytes_recieved]='\0';

		strcpy(FileStructure[i].name,recv_data);
		
		if(strcmp(type,"tcp")==0)
			bytes_recieved = recv(sock,recv_data,1024,0);
		else
			bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
		recv_data[bytes_recieved]='\0';
		strcpy(FileStructure[i].type,recv_data);
		/********* recieving File size packet ************/
		if(strcmp(type,"tcp")==0)
			recv(sock,&recv_data_int,sizeof(int),0);
		else
			recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,sock_size);

		FileStructure[i].size = recv_data_int;

		/********* recieving Last Modified time packet ************/
		if(strcmp(type,"tcp")==0)
			bytes_recieved = recv(sock,recv_data,1024,0);
		else
			bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
		recv_data[bytes_recieved]='\0';
		strcpy(FileStructure[i].time,recv_data);
	}
	if(strcmp(command[1],"ShortList")==0)
	{
		char time1[100];
		char time2[100];
		if(command_count==6)
		{
			int i,j=0;
			for(i=0;i<strlen(command[2]);i++)
			{
				if(i!=4 && i!=7 && i!=10)
				{
					time1[j]=command[2][i];
					time2[j]=command[4][i];
					j++;
				}
			}
			for(;i<(strlen(command[2])+strlen(command[3]));i++)
			{
				if(i!=(strlen(command[2])+2) && i!=(strlen(command[2])+5) && i!=(strlen(command[2])+8))
				{
					time1[j]=command[3][i-strlen(command[2])];
					time2[j]=command[5][i-strlen(command[2])];
					j++;
				}
			}
			time1[j]='\0';
			time2[j]='\0';
			//					strcpy(time1,concat(command[2],command[3]));
			//					strcpy(time2,concat(command[4],command[5]));
			for(i=0;i<file_count;i++)
			{
				/**** compare the time stamps and select the proper files to show ***/
				parse_time(FileStructure[i].time);
				if(strcmp(global_time,time1)>=0 && strcmp(time2,global_time)>=0)
				{
					printf("\n###################\n");
					printf("\n###################\n");
					printf("Filename : %s		Filesize = %d\n		Filetype = %s\n		time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
				}
			}
			printf("\n###################\n");
			printf("\n###################\n");
		}
		else
			printf("Error in timeStamps\n");
	}
	else if(command_count>1 && strcmp(command[1],"LongList")==0)
	{
		printf("enter\n");
		/************* Long Listing of recieved files *********/
		for(int i=0;i<file_count;i++)
		{
			printf("\n###################\n");
			printf("\n###################\n");
			printf("Filename : %s		Filesize = %d		Filetype = %s\n		time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
		}
			printf("\n###################\n");
			printf("\n###################\n");
	}
	else if(command_count>1 && strcmp(command[1],"regEx")==0)
	{
		if(strlen(command[2])>0)
		{
			/*********** recieved special regEx packets *****/
			if(strcmp(type,"tcp")==0)
				bytes_recieved = recv(sock,recv_data,1024,0);
			else
				bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
			recv_data[bytes_recieved]='\0';
			while(strcmp(recv_data,"End of File")!=0)
			{
				for(int i=0;i<file_count;i++)
				{
					if(strcmp(recv_data,FileStructure[i].name)==0)
					{
						printf("\n###################\n");
						printf("\n###################\n");
						printf("Filename : %s		Filesize = %d		Filetype = %s\n		time=%s\n",FileStructure[i].name,FileStructure[i].size,FileStructure[i].type,FileStructure[i].time);
					}
				}
				if(strcmp(type,"tcp")==0)
					bytes_recieved = recv(sock,recv_data,1024,0);
				else
					bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
				recv_data[bytes_recieved]='\0';
			}
			printf("\n###################\n");
			printf("\n###################\n");
		}
		else
			printf("Argument for regEx is Missing \n");
	}
	else
		printf("Arguments Missing\n");
}


void download(int sock, char type[],struct sockaddr_in server_addr, socklen_t * sock_size, int recv_data_int, char recv_data[],int bytes_recieved, char md5[], char recv_md5[])
{
	if(command_count < 2)
	{
		printf("arguments missing\n");
		scan_input();
	}
	else
	{
		char * sen;
		if(strcmp(type,"tcp")==0)
		{
			sen=concat(complete_command,command[1]);
			send(sock,sen,strlen(sen),0);
			bytes_recieved = recv(sock,recv_data,1024,0);
		}
		else
		{
			sen=concat(complete_command,command[1]);
			sendto(sock,sen,strlen(sen),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
			bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
		}
		recv_data[bytes_recieved]='\0';

		if(strcmp(recv_data,"file doesn't exist")!=0)
		{
			if(strcmp(type,"tcp")==0)
			{
				recv(sock,recv_data,MD5_LEN+1,0);
				strcpy(recv_md5,recv_data);
				recv(sock,&recv_data_int,sizeof(recv_data_int),0);
				bytes_recieved = recv(sock,recv_data,1024,0);
				
				FILE *fp;
				fp = fopen(command[1],"w");
				printf("<");
				while(strcmp(recv_data,"End of File")!=0)
				{
					printf(".");
					for(int i=0;i<recv_data_int;i++)
						fprintf(fp,"%c",recv_data[i]);
					/********* recieving packet size************/
					if(strcmp(type,"tcp")==0)
						recv(sock,&recv_data_int,sizeof(int),0);
					else
						recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,sock_size);
					/********* recievin packet data**************/
					if(strcmp(type,"tcp")==0)
						bytes_recieved = recv(sock,recv_data,1024,0);
					else
						bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
					recv_data[bytes_recieved]='\0';
				}
				fclose(fp);
			}
			else
			{
				recvfrom(sock,recv_data,MD5_LEN+1,0,(struct sockaddr *)&server_addr,sock_size);
				strcpy(recv_md5,recv_data);
				recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,sock_size);
				bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
				FILE *fp;
				fp = fopen(command[1],"w");
				printf("Open FILE %d\n",recv_data_int);
				printf("<");
				while(strcmp(recv_data,"End of File")!=0)
				{
					printf(".");
					for(int i=0;i<recv_data_int;i++)
						fprintf(fp,"%c",recv_data[i]);
					/********* recieving packet size************/
					if(strcmp(type,"tcp")==0)
						recv(sock,&recv_data_int,sizeof(int),0);
					else
						recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,sock_size);
					/********* recievin packet data**************/
					if(strcmp(type,"tcp")==0)
						bytes_recieved = recv(sock,recv_data,1024,0);
					else
						bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
					recv_data[bytes_recieved]='\0';
				}
				fclose(fp);
			}
			printf(">\n");
			printf("checking for the md5sum\n");
			printf("md5sum of the file to be downloaded = %s\n",recv_md5);
			if (!CalcFileMD5(command[1], md5)) {
				puts("Error occured in md5sum! :(                          [Fail]");
			} 
			else {
				printf("md5sum of the file recieved: %s                    [OK]\n", md5);
			}
			if(strcmp(md5,recv_md5)==0){
				printf("md5sum for the file matched                        [OK]\n");
				printf("File Download completed                            [OK]\n");
			}
			else
				printf("md5 check sum error                                 [Fail]\n");
		}
		else
			printf("No such file or directory found on the remote host              [Fail]\n");
	}
}

int client_connect(int connect_port_no,char * type){
	int sock;  
	char x;
	struct hostent *host;
	struct sockaddr_in server_addr;

	host = gethostbyname("127.0.0.1");
	if(strcmp(type,"tcp")==0){
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}
	else if(strcmp(type,"udp")==0){
		if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			perror("Socket");
			return 1;
		}
	}
	else 
	{
		perror("Invalid Protocol");
		return 1;
	}

	server_addr.sin_family = AF_INET;     
	server_addr.sin_port = htons(connect_port_no);   
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(server_addr.sin_zero),8); 
	int sin_size = sizeof(struct sockaddr_in);
	socklen_t * sock_size = (socklen_t *) &sin_size;
	if(strcmp(type,"tcp")==0){
		if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1) {
			perror("Connect");
			return 2;
		}
		printf("\n CLIENT : Connected to Port No %d\n",connect_port_no);
	}
	scanf("%c",&x);
	scan_input();
	execute(sock, server_addr,type, sock_size);
	return 0;
}

void execute(int sock, struct sockaddr_in server_addr, char * type, socklen_t * sock_size)
{
	int recv_data_int, bytes_recieved;
	char send_data[1024],recv_data[1024];
	fs FileStructure[1000];
	char md5[MD5_LEN + 1];
	char recv_md5[MD5_LEN + 1];

	while(strcmp(command[0],"Exit")!=0 && strcmp(command[0],"q")!=0 && strcmp(command[0],"Q")!=0 && strcmp(command[0],"exit")!=0 )
	{
		if(strcmp(command[0],"FileDownload")==0)
		{
			download(sock,type,server_addr,sock_size,recv_data_int,recv_data,bytes_recieved,md5,recv_md5);
			if(strcmp(command[0],"FileDownload")!=0)
			{
				if(command_count < 2)
				{
					printf("arguments missing\n");
					scan_input();
					continue;
				}
				else
				{
					if(strcmp(type,"tcp")==0)
					{
						char * sen;
						sen=concat(complete_command,command[1]);
						send(sock,sen,strlen(sen),0);
						bytes_recieved = recv(sock,recv_data,1024,0);
					}
					else
					{
						sendto(sock,complete_command,strlen(complete_command),0,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
						bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
					}
					recv_data[bytes_recieved]='\0';

					if(strcmp(recv_data,"file doesn't exist")!=0)
					{
						if(strcmp(type,"tcp")==0)
						{
							recv(sock,recv_data,MD5_LEN+1,0);
							strcpy(recv_md5,recv_data);
							recv(sock,&recv_data_int,sizeof(recv_data_int),0);
							bytes_recieved = recv(sock,recv_data,1024,0);
							/*					if(strcmp(type,"tcp")==0)
												recv(sock,recv_data,MD5_LEN+1,0);
												else
												recvfrom(sock,recv_data,MD5_LEN+1,0,(struct sockaddr *)&server_addr,sock_size);
												strcpy(recv_md5,recv_data);
												if(strcmp(type,"tcp")==0)
												recv(sock,&recv_data_int,sizeof(recv_data_int),0);
												else
												recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,sock_size);
												if(strcmp(type,"tcp")==0)
												bytes_recieved = recv(sock,recv_data,1024,0);
												else
												bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
							 */
							FILE *fp;
							fp = fopen(command[1],"w");
							printf("<");
							while(strcmp(recv_data,"End of File")!=0)
							{
								printf(".");
								for(int i=0;i<recv_data_int;i++)
									fprintf(fp,"%c",recv_data[i]);
								/********* recieving packet size************/
								if(strcmp(type,"tcp")==0)
									recv(sock,&recv_data_int,sizeof(int),0);
								else
									recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,sock_size);
								/********* recievin packet data**************/
								if(strcmp(type,"tcp")==0)
									bytes_recieved = recv(sock,recv_data,1024,0);
								else
									bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
								recv_data[bytes_recieved]='\0';
							}
						}
						else
						{
							printf("IN UDP\n");
							recvfrom(sock,recv_data,MD5_LEN+1,0,(struct sockaddr *)&server_addr,sock_size);
							strcpy(recv_md5,recv_data);
							recvfrom(sock,&recv_data_int,sizeof(recv_data_int),0,(struct sockaddr *)&server_addr,sock_size);
							bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
							FILE *fp;
							fp = fopen(command[1],"w");
							printf("OPENFILE %d\n",recv_data_int);
							printf("<");
							while(strcmp(recv_data,"End of File")!=0)
							{
								printf(".");
								for(int i=0;i<recv_data_int;i++)
									fprintf(fp,"%c",recv_data[i]);
								/********* recieving packet size************/
								if(strcmp(type,"tcp")==0)
									recv(sock,&recv_data_int,sizeof(int),0);
								else
									recvfrom(sock,&recv_data_int,sizeof(int),0,(struct sockaddr *)&server_addr,sock_size);
								/********* recievin packet data**************/
								if(strcmp(type,"tcp")==0)
									bytes_recieved = recv(sock,recv_data,1024,0);
								else
									bytes_recieved = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&server_addr,sock_size);
								recv_data[bytes_recieved]='\0';
							}
							printf("End of FILE UDP\n");
						}
						printf(">\n");
						printf("checking for the md5sum\n");
						printf("md5sum of the file to be downloaded = %s\n",recv_md5);
						if (!CalcFileMD5(command[1], md5)) {
							puts("Error occured in md5sum! :(                          [Fail]");
						} 
						else {
							printf("md5sum of the file recieved: %s                    [OK]\n", md5);
						}
						if(strcmp(md5,recv_md5)==0){
							printf("md5sum for the file matched                        [OK]\n");
							printf("File Download completed                            [OK]\n");
						}
						else
							printf("md5 check sum error                                 [Fail]\n");
					}
					else
						printf("No such file or directory found on the remote host              [Fail]\n");
				}
			}
		}
		else if(strcmp(command[0],"IndexGet")==0)
		{
			IndexGet(sock,type,recv_data_int,server_addr,sock_size,bytes_recieved,recv_data,FileStructure);
		}
		else{
			if(strcmp(complete_command,"")!=0)
				printf("INVALID COMMAND\n");
		}
		scan_input();
	}


}


int main(){
	FILE *upload_file;
	upload_file = fopen("upload_command","w");
	fprintf(upload_file,"allow");
	fclose(upload_file);
	int server_port_no;
	int connect_port_no;
	char *type = (char *)malloc(sizeof(char) * 100);
	printf("Give port no to which you want to send the data (>1024): ");
	scanf("%d",&connect_port_no);
	printf("Type of transfer protocol(tcp/udp): ");
	scanf("%s",type);
	while(client_connect(connect_port_no,type)>0){
		sleep(1);
	}
	return 0;
}
