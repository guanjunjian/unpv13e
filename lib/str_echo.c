#include	"unp.h"

//作用：处理每个客户的服务：从客户读入数据，并把它们回射给客户
void
str_echo(int sockfd)
{
	ssize_t		n;
	char		buf[MAXLINE];

again:
    //read从套接字读入数据,如果客户关闭连接，那么接收到客户的FIN将导致服务器子进程的read返回0，否则返回读取到的字节数
	while ( (n = read(sockfd, buf, MAXLINE)) > 0)
        //Writen把buf中的内容回射给客户
		Writen(sockfd, buf, n);

	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		err_sys("str_echo: read error");
}
