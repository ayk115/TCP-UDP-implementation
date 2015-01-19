/*
FTP Assignment 
*/
//Shikher Somal -- 201201091
//Ayush Khandelwal -- 201202069

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


#define LEN_OF_PATH 256

#define MD5_HASH_LEN 32
#define CURRENT_DIR "./"
typedef struct file{
	char name[1024];
	char time[1024];
	int size;
	char type[200];
	char filemd5[MD5_HASH_LEN+1];
}fs;


#define STR(name) VALUE_STRING(name)
//Global Variable for server side
char server_send_data [1024],server_recv_data[1024];
char recv_complete_command[1024];

char recv_command[32][32];
int recv_command_count=0;

//Function Declarations

int get_directory_info(DIR *d_error);

char *strings_conctn_func(char c1[],char c2[]){
	char *ans = (char *)malloc(sizeof(char)*100);
	for(int i=0;i<strlen(c1);i++)ans[i]=c1[i];
	ans[strlen(c1)]=' ';
	for(int i=strlen(c1)+1;i<strlen(c1)+strlen(c2)+1;i++)ans[i]=c2[i-strlen(c1)-1];
	ans[strlen(c1)+strlen(c2)+1]='\0';
	return ans;
}

void update_file_structure(){
	DIR *d_error;
	

	d_error = opendir (CURRENT_DIR);
	int total_files=0;
	if (d_error != NULL)
	{
		
		total_files=get_directory_info(d_error);
		printf("Total Bytes of Data %d\n",total_files);
	}
	else
		puts ("Permission denied or Not able to open folder");
}

// Global declarations
fs filestructure_of_server[1000];

int Server_file_count =0;

char global_time[100];

int CalcFileMD5(char *file_name, char *md5_sum);
#define VALUE_STRING(val) #val
int get_directory_info(DIR *d_error);


void parse(){
	char c;
	int count=0;

	for(int i=0;i<strlen(server_recv_data);i++)
		recv_complete_command[count++]=server_recv_data[i];

	recv_complete_command[count++]='\0';
	recv_command_count=0;

	int count2=0;
	for(int i=0;i<strlen(recv_complete_command);i++){
		if(recv_complete_command[i]==' '){
			recv_command[recv_command_count][count2++]='\0';
			recv_command_count++;
			count2=0;
			continue;
		}   
		recv_command[recv_command_count][count2]=recv_complete_command[i];
		count2++;
	}  
	recv_command[recv_command_count][count2++]='\0';
	recv_command_count++;
}
int get_directory_info(DIR *d_error)
{
	struct dirent * Store_data[1000];	
	printf("Updating Data Structure of Directory\n");
	Server_file_count = 0;
	struct dirent *ep;
	int flag=0;
	int bytes=0;
	for(bytes=0;ep = readdir (d_error);bytes++){
			
			flag=1;			
			strcpy(filestructure_of_server[Server_file_count].name,ep->d_name);
			
			struct stat st;
			stat(ep->d_name, &st);
			
			int size = st.st_size;
			char terminal_command[100];
			Store_data[0]=	ep;
			strcpy(terminal_command,strings_conctn_func("file ",filestructure_of_server[Server_file_count].name));
			system(strings_conctn_func(terminal_command,">fileType"));
			ifstream input;

			string line;
			input.open("fileType");

			getline(input,line);
			input.close();


			strcpy(filestructure_of_server[Server_file_count].type,line.c_str());
			filestructure_of_server[Server_file_count].size= size;


			strcpy(filestructure_of_server[Server_file_count].time,ctime(&st.st_mtime));

			Server_file_count++;
	}
	Store_data[999]=ep;
	if(Store_data[999]==NULL)
	{
		printf("Successfull end of data\n");
	}
	(void) closedir (d_error);
	return bytes;
}

void Download_helper_main(char * type,int main_socket, int main_connection, int total_bytes_got,struct sockaddr_in * cli_address,char md5[])	
{
						char ch;
						FILE *fp;
						fp = fopen(recv_command[1],"r");
						int count;
						if(strcmp(type,"tcp")==0)
							send(main_connection,md5,MD5_HASH_LEN+1,0);
						else
							sendto(main_socket,md5,MD5_HASH_LEN+1,0,(struct sockaddr *)&cli_address,sizeof(struct sockaddr));
						while(fscanf(fp,"%c",&ch)!=EOF){
							count=0;
							server_send_data[count++]=ch;
							while(count<1024 && fscanf(fp,"%c",&ch)!=EOF){
								server_send_data[count++]=ch;
							}
							if(strcmp(type,"tcp")==0)
								send(main_connection,&count,sizeof(int),0);
							else
								sendto(main_socket,&count,sizeof(int),0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));


							if(strcmp(type,"tcp")==0)
								send(main_connection,server_send_data,1024,0);
							else
								sendto(main_socket,server_send_data,1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));

						}
}


void IndexGet(char * type,int main_socket, int main_connection, int total_bytes_got,struct sockaddr_in * cli_address)
{
					
					/*******************************************/

					if(strcmp(type,"tcp")==0)
						send(main_connection,&Server_file_count,sizeof(int),0);
					else
						sendto(main_socket,&Server_file_count,sizeof(int),0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
					if(strcmp(type,"tcp")==0)
					{
					int i=0;
					while(i<Server_file_count)
					{
						
						send(main_connection,filestructure_of_server[i].name,1024,0);
						send(main_connection,filestructure_of_server[i].type,1024,0);
						send(main_connection,&filestructure_of_server[i].size,sizeof(int),0);
						send(main_connection,filestructure_of_server[i].time,1024,0);
						i++;
					}
					}
					else if(strcmp(type,"udp")==0)
					{
					int i=0;
					while(i<Server_file_count)
					{
						
						sendto(main_socket,filestructure_of_server[i].name,1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
						sendto(main_socket,filestructure_of_server[i].type,1024,0,(struct sockaddr *)&cli_address,sizeof(struct sockaddr));
						sendto(main_socket,&filestructure_of_server[i].size,sizeof(int),0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
						sendto(main_socket,filestructure_of_server[i].time,1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
						i++;
					}
					
					}
					if(recv_command_count>1){
						if(strcmp(recv_command[1],"regEx")==0){
							char terminal_command[100];
							strcpy (terminal_command,strings_conctn_func("ls ",recv_command[2]));
							system(strings_conctn_func(terminal_command,">out"));
							FILE * write_file = fopen("out","r");
							int bytes=0;
							if(strcmp(type,"tcp")==0)
							{
							printf("TCP\n");
							for(bytes=0;fscanf(write_file,"%s",&server_send_data)!=EOF;bytes++){
								
								
									send(main_connection,server_send_data,1024,0);
							}
							}
							else if(strcmp(type,"udp"))
							{
							for(bytes=0;fscanf(write_file,"%s",&server_send_data)!=EOF;bytes++){
								
								
									sendto(main_socket,server_send_data,1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
							}
							
							}
							printf("%d\n",bytes);
							if(strcmp(type,"tcp")==0)
								send(main_connection,"End of File\0",1024,0);
							else if(strcmp(type,"udp")==0)
								sendto(main_socket,"End of File\0",1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
						}
					}
}

int CalcFileMD5(char *file_name, char *md5_sum)
{
	#define MD5SUM_CMD_FMT "ms5hash %." STR(LEN_OF_PATH) "s 2>/dev/null"
	char cmd[LEN_OF_PATH + sizeof (MD5SUM_CMD_FMT)];
	sprintf(cmd, MD5SUM_CMD_FMT, file_name);
	#undef MD5SUM_CMD_FMT
	printf("%s\n",file_name);	
	printf("%s\n",cmd);
	FILE *p = popen(cmd, "r");
	if (p == NULL) return 0;

	int i=0, ch;
	while ( i < MD5_HASH_LEN && isxdigit(ch = fgetc(p))) {
		*md5_sum++ = ch;
		i++;
	}

	*md5_sum = '\0';
	pclose(p);
	return i == MD5_HASH_LEN;
}

long getFileSize(FILE *file)
{
	long lCurPos, lEndPos;
	lCurPos = ftell(file);
	cout<<endl<<"lcurPos  = "<<lCurPos<<endl;
	fseek(file, 0, 2);
	lEndPos = ftell(file);
	cout<<"lEndPos = "<<lEndPos;
	fseek(file, lCurPos, 0);
	return lEndPos;
}


int server_code(int port_of_server,char *type){
	int garbage_int=0;
	char md5[MD5_HASH_LEN + 1];
	char recv_md5[MD5_HASH_LEN + 1];

	int main_socket, main_connection, total_bytes_got ;  

	struct sockaddr_in serv_address,cli_address;    
	int sin_size;

	if(strcmp(type,"tcp")==0){

		if ((main_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			

			perror("Socket Error has occured");
			return 1;
		}
	}
	if(strcmp(type,"udp")==0){

		if ((main_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			

			perror("Socket Error has Occured");
			return 1;
		}
	}
        

//s
        memset(&serv_address, 0, sizeof(serv_address));
//s
	serv_address.sin_family = AF_INET;         
	serv_address.sin_port = htons(port_of_server);     
	serv_address.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(serv_address.sin_zero),8); 
	//Socket binding
	if (bind(main_socket, (struct sockaddr *)&serv_address, sizeof(struct sockaddr))
			== -1) {
		perror("Unable to bind");
		return 2;
	}
	// print socket
	if(strcmp(type,"tcp")==0){
		if (listen(main_socket, 5) == -1) {
			perror("Listen");
			return 3;
		}
	}
	// cout << socket
	if(strcmp(type,"tcp")==0)
		printf("\nTCPServer is waiting on the following port number: %d\n $ ",port_of_server);
	else if(strcmp(type,"udp")==0)
		printf("\nUDPServer is waiting on the following port number: %d\n $ ",port_of_server);

	fflush(stdout);


	while(1){  

		sin_size = sizeof(struct sockaddr_in);
		socklen_t * lol = (socklen_t *) &sin_size;
		if(strcmp(type,"tcp")==0){
			main_connection = accept(main_socket, (struct sockaddr *)&cli_address,lol);
			printf("\n Connection has been received from (%s , %d)\n $ ", inet_ntoa(cli_address.sin_addr),ntohs(cli_address.sin_port));
		}

		// Recieving Commands
		while (1){
			if(strcmp(type,"tcp")==0)
				total_bytes_got = recv(main_connection,server_recv_data,1024,0);
			if(strcmp(type,"udp")==0)
				total_bytes_got = recvfrom(main_socket,server_recv_data,1024,0,(struct sockaddr *)&cli_address, lol);
			server_recv_data[total_bytes_got] = '\0';

			/*** parsing of commands **/ 
                        	
			parse();
			// Checking bytes
			if(total_bytes_got==0){
				printf("\nConnection has been closed by remote client\n $ ");
				close(main_connection);
				break;
			}
			if(strcmp(server_recv_data , "Exit") == 0 || strcmp(server_recv_data , "exit") == 0){
				printf("Connection has been closed by remote client\n $ ");

				close(main_connection);
				break;
			}
			else{
				printf("\n RECIEVED DATA = %s \n $ " , recv_complete_command);

				if(strcmp(recv_command[0],"FileDownload")==0){
					ifstream ifile(recv_command[1]);
					if(ifile){
						//Check if FileExists
						if(strcmp(type,"tcp")==0)
							send(main_connection,"file exists",1024,0);
						else
							sendto(main_socket,"file exists",1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));

						if (!CalcFileMD5(recv_command[1], md5)) {
							puts("Error occured in ms5hash! :(");
						} 
						else {
							printf("\nms5hash: %s\n", md5);
						}
						Download_helper_main(type,main_socket,main_connection,total_bytes_got,&cli_address,md5);
						
						if(strcmp(type,"tcp")==0)
							send(main_connection,&garbage_int,sizeof(int),0);
						else
							sendto(main_socket,&garbage_int,sizeof(int),0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));

						if(strcmp(type,"tcp")==0)
							send(main_connection,"End of File",1024,0);
						else
							sendto(main_socket,"End of File",1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
					}
					else{
						
						if(strcmp(type,"tcp")==0)
							send(main_connection,"file doesn't exist",1024,0);
						else
							sendto(main_socket,"file doesn't exist",1024,0,(struct sockaddr *)&cli_address, sizeof(struct sockaddr));
					}
				}
				if(strcmp(recv_command[0],"IndexGet")==0){
					
					update_file_structure();
                                        IndexGet(type,main_socket,main_connection,total_bytes_got,&cli_address);
					
				}
			
			}
		}
		fflush(stdout);
	}
	close(main_socket);
	return 0;
}


int main(){
	FILE *upload_file;
	upload_file = fopen("upload_command","w");
	fprintf(upload_file,"allow");
	fclose(upload_file);
	int port_of_server;
	int connect_port_no;
	char *type = (char *)malloc(sizeof(char) * 100);
	printf(">>>>>>>>>>>>>>>  ENTER PORT NUMBER >>>>>>>>>>>>>>>>");
	scanf("%d",&port_of_server);
	printf("Choose Protocol (tcp/udp): ");
	scanf("%s",type);
	server_code(port_of_server,type);
	return 0;
}
