#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/kernel.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#define SERVER_PORT 5000
#define BUFF_LEN 1024
#define NIPQUAD_FMT "%u.%u.%u.%u"
#define NIPQUAD(addr)\
	((unsigned char*)&addr)[0], ((unsigned char*)&addr)[1], ((unsigned char*)&addr)[2], ((unsigned char*)&addr)[3]


void handle_udp_msg(int fd)
{
	char buf[BUFF_LEN];
	char clientname[100];
	socklen_t len;
	int count;
	struct sockaddr_in clent_addr;

	while(1)
	{
		memset(buf, 0, BUFF_LEN);
		len = sizeof(clent_addr);
		count = recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&clent_addr, &len);
		if(count == -1)
		{
			printf("recieve data fail!\n");
			return;
		}
		printf("[INPUT]: " NIPQUAD_FMT ":%d : %s\n", NIPQUAD(clent_addr.sin_addr.s_addr), htons(clent_addr.sin_port), buf);

		if(strcmp(buf, "register") == 0)
		{
			memset(buf, 0, BUFF_LEN);
			sprintf(buf, "registered");
			printf("[OUTPUT]: " NIPQUAD_FMT ":%d : %s\n", NIPQUAD(clent_addr.sin_addr.s_addr), htons(clent_addr.sin_port), buf);
			sprintf(clientname,"clients/" NIPQUAD_FMT, NIPQUAD(clent_addr.sin_addr.s_addr));
			int ffd = open(clientname, O_CREAT, 0755);
			close(ffd);
			sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr*)&clent_addr, len);
		}
	}
}
int init()
{
	int server_fd, ret;
	struct sockaddr_in ser_addr;

	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(server_fd < 0)
	{
		printf("create socket fail!\n");
		return -1;
	}
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(SERVER_PORT);
	ret = bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
	if(ret < 0)
	{
		printf("socket bind fail!\n");
		return -1;
	}

	return server_fd;

}

void Stop(int signo)
{
	char *dir_name = "clients";
	DIR  *dirp;
	char strTmpPath[500] = "\0";
	struct dirent *dp;
	dirp = opendir(dir_name);
	while((dp = readdir(dirp)) != NULL){
		if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0){
			continue;
		}
		sprintf(strTmpPath, "%s/%s", dir_name, dp->d_name);
		int n = remove(strTmpPath);
	}
	closedir(dirp);
	_exit(0);
}
int main(int argc, char* argv[])
{
	signal(SIGINT, Stop); 	//退出时删除clients里的所有文件
	int fd = init();
	handle_udp_msg(fd);
	close(fd);
	return 0;
}
