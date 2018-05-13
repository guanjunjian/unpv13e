#include	"web.h"

void
write_get_cmd(struct file *fptr)
{
	int		n;
	char	line[MAXLINE];

	//构造命令
	n = snprintf(line, sizeof(line), GET_CMD, fptr->f_name);
	//将命令写到套接字中
	Writen(fptr->f_fd, line, n);
	printf("wrote %d bytes for %s\n", n, fptr->f_name);

	//设置F_READING，它同时清除F_CONNECTING标志
	//该命令向main函数主循环指出：本描述符已准备好提供输入
	fptr->f_flags = F_READING;			/* clears F_CONNECTING */

	//在读描述符集中打开与本描述符对应的位
	FD_SET(fptr->f_fd, &rset);			/* will read server's reply */
	if (fptr->f_fd > maxfd)
		maxfd = fptr->f_fd;
}
