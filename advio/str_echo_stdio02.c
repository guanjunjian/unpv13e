#include	"unp.h"

void
str_echo(int sockfd)
{
	char		line[MAXLINE];
	FILE		*fpin, *fpout;

	//创建输入流
	fpin = Fdopen(sockfd, "r");
	//创建输出流
	fpout = Fdopen(sockfd, "w");

	//读到FIN，则返回空指针
	while (Fgets(line, MAXLINE, fpin) != NULL)
		Fputs(line, fpout);
}
