#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/kernel.h>
#include <fcntl.h>

#define SERVER_PORT 5000
#define BUFF_LEN 1024
#define NIPQUAD_FMT "%u.%u.%u.%u"
#define NIPQUAD_FMTF "%u%u%u%u"
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
		printf("[INPUT]: " NIPQUAD_FMT " : %s\n", NIPQUAD(clent_addr.sin_addr.s_addr), buf);

		if(strcmp(buf, "register") == 0)
		{
			memset(buf, 0, BUFF_LEN);
			sprintf(buf, "registered");
			printf("[OUTPUT]: " NIPQUAD_FMT " : %s\n", NIPQUAD(clent_addr.sin_addr.s_addr), buf);
			sprintf(clientname,"clients/" NIPQUAD_FMTF, NIPQUAD(clent_addr.sin_addr.s_addr));
			open(clientname, O_CREAT, 0755);
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

int main(int argc, char* argv[])
{
	int fd = init();
	handle_udp_msg(fd);
	close(fd);
	return 0;
}
