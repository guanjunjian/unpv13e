/* include readable_timeo */
#include	"unp.h"

int
readable_timeo(int fd, int sec)
{
	fd_set			rset;
	struct timeval	tv;

	FD_ZERO(&rset);
	//在读描述符集中打开与调用者给定描述符对应的位
	FD_SET(fd, &rset);

	//把调用者给定的等待秒数设置在一个timeval结构中
	tv.tv_sec = sec;
	tv.tv_usec = 0;

	//select等待该描述符变为可读，或发生超时
	//出错时为-1
	//超时发生时为0
	//正常则返回正值，为已就绪描述符的数目
	return(select(fd+1, &rset, NULL, NULL, &tv));
		/* 4> 0 if descriptor is readable */
}
/* end readable_timeo */

int
Readable_timeo(int fd, int sec)
{
	int		n;

	if ( (n = readable_timeo(fd, sec)) < 0)
		err_sys("readable_timeo error");
	return(n);
}
