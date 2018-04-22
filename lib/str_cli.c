#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	char	sendline[MAXLINE], recvline[MAXLINE];
	//Fgets从客户端读入一行数据
	//当遇到文件结束符或错误，fgets（库函数）将返回一个空指针，于是客户端循环终止
	//Fgets包裹fgets，检查是否发送错误，发送则中指程序，因此Fgets只有遇到文件结束符时才返回一个空指针
	while (Fgets(sendline, MAXLINE, fp) != NULL) {
		//Writen把从客户端读到的数据发送到服务端
		Writen(sockfd, sendline, strlen(sendline));
		//Readline读取从服务器回射的数据
		if (Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli: server terminated prematurely");
		//Fputs把它写到标准输出
		Fputs(recvline, stdout);
	}
}
