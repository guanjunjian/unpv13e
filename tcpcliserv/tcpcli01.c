#include	"unp.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");
	
    //1.创建套接字
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	
    //2.创建服务器地址结构
    //2.1将地址结构清空
	bzero(&servaddr, sizeof(servaddr));
    //2.2.设置协议族
	servaddr.sin_family = AF_INET;
    //2.3.绑定端口，#define SERV_PORT 9877
	servaddr.sin_port = htons(SERV_PORT); //端口号SERV_PORT(9877)
    //2.4.根据main函数参数argv[1]获取服务器ip地址
    //Inet_pton的作用：ASCII字符串--->套接字地址结构中的二进制值
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	
    //3.发起TCP 3次握手建立连接
	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	
    //4.从输入按行读取文本，发送至服务器，接收并显示回显
	str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}