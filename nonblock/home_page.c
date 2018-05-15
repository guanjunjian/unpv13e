#include	"web.h"

void
home_page(const char *host, const char *fname)
{
	int		fd, n;
	char	line[MAXLINE];

	//建立一个与服务器的连接
	//阻塞式connect
	fd = Tcp_connect(host, SERV);	/* blocking connect() */

	//发出一个HTTP GET命令以获取主页（文件名经常是/）
	n = snprintf(line, sizeof(line), GET_CMD, fname);
	Writen(fd, line, n);

	for ( ; ; ) {
		//读取应答（不对应答做任何操作）
		if ( (n = Read(fd, line, MAXLINE)) == 0)
			break;		/* server closed connection */

		printf("read %d bytes of home page\n", n);
		/* do whatever with data */
	}
	printf("end-of-file on home page\n");
	//关闭连接
	Close(fd);
}
