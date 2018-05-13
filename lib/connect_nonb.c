#include	"unp.h"

int
connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	int				flags, n, error;
	socklen_t		len;
	fd_set			rset, wset;
	struct timeval	tval;

	flags = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	//期待的错误是EINPROGRESS
	//connect返回的任何其他错误返回给函数的调用者
	if ( (n = connect(sockfd, saptr, salen)) < 0)
		if (errno != EINPROGRESS)
			return(-1);

	/* Do whatever we want while the connect is taking place. */

	//如果非阻塞返回0，那么连接已经建立
	//即服务器处于客户所在主机时可能发生这种情况
	if (n == 0)
		goto done;	/* connect completed immediately */

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	//如果调用者将nsec设为0，则需要将select的最后一个参数设为NULL
	//表示使用默认超时时间
	if ( (n = Select(sockfd+1, &rset, &wset, NULL,
					 nsec ? &tval : NULL)) == 0) {
		close(sockfd);		/* timeout */
		errno = ETIMEDOUT;
		return(-1);
	}

	//如果描述符变为可读或可写
	if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		//调用getsockopt获取待处理错误
		//如果连接建立成功，该值为0，则不做处理
		//如果连接建立发生错误，该值就是对应连接错误的errno
		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return(-1);			/* Solaris pending error */
	} else
		err_quit("select error: sockfd not set");

done:
	//恢复套接字的文件状态标志
	Fcntl(sockfd, F_SETFL, flags);	/* restore file status flags */
	
	//如果自getsockopt返回的errno变量为非0值，
	//把该值存入errno，函数本身返回-1
	if (error) {
		close(sockfd);		/* just in case */
		errno = error;
		return(-1);
	}
	return(0);
}
