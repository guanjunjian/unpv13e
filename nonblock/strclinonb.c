/* include nonb1 */
#include	"unp.h"

void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, val, stdineof;
	ssize_t		n, nwritten;
	fd_set		rset, wset;
	char		to[MAXLINE], fr[MAXLINE];
	char		*toiptr, *tooptr, *friptr, *froptr;

	//把套接字设为非阻塞
	val = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	//把标准输入设为非阻塞
	val = Fcntl(STDIN_FILENO, F_GETFL, 0);
	Fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

	//把标准输出设为非阻塞
	val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
	Fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	//初始化缓冲区指针
	toiptr = tooptr = to;	/* initialize buffer pointers */
	friptr = froptr = fr;
	stdineof = 0;

	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
	for ( ; ; ) {
		//初始化读描述符集
		FD_ZERO(&rset);
		//初始化写描述符集
		FD_ZERO(&wset);
		//stdineof == 0 表示标准输入尚未读到EOF
		//toiptr < &to[MAXLINE] 表示还有容纳从标准输入读入的缓冲空间
		//打开读描述符集中对应标准输入的位
		if (stdineof == 0 && toiptr < &to[MAXLINE])
			FD_SET(STDIN_FILENO, &rset);	/* read from stdin */
		//如果还有容纳从套接字读入的缓冲空间
		//打开读描述符集中对应套接字的位
		if (friptr < &fr[MAXLINE])
			FD_SET(sockfd, &rset);			/* read from socket */
		//如果还有要写到套接字的数据
		//打开写描述符集中套接字对应的位
		if (tooptr != toiptr)
			FD_SET(sockfd, &wset);			/* data to write to socket */
		//如果还有要写入到标准输出的数据
		//打开写描述符集中标准输出对应的位
		if (froptr != friptr)
			FD_SET(STDOUT_FILENO, &wset);	/* data to write to stdout */

		Select(maxfdp1, &rset, &wset, NULL, NULL);
/* end nonb1 */
/* include nonb2 */
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			//如果标准输入读就绪，将数据写入to缓冲区中
			if ( (n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
				//如果发生EWOULDBLOCK错误，则忽略它（这里没有处理该错误的代码）
				//通常情况下这种条件“不应该发生”，因为这意味着，select告知我们
				//相应描述符可读，然而read该描述符却返回EWOULDBLOCK错误

				//如果错误不是EWOULDBLOCK错误，则输出该错误
				if (errno != EWOULDBLOCK)
					err_sys("read error on stdin");

			} else if (n == 0) {
#ifdef	VOL2
				fprintf(stderr, "%s: EOF on stdin\n", gf_time());
#endif			//如果read返回0，那么标准输入处理就此结束，设置stdineof标志位1
				stdineof = 1;			/* all done with stdin */
				//如果在to缓冲区中不再有数据要发送，就调用shutdown发送FIN到服务器
				//如果还有数据要发送，FIN的发送就得推迟到缓冲区中数据已写到套接字之后
				if (tooptr == toiptr)
					Shutdown(sockfd, SHUT_WR);/* send FIN */

			} else {
#ifdef	VOL2
				fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
#endif			//正常情况
				//当read返回数据时，我们增加toiptr
				//打开写描述符集中与套接字对应的位
				//当套接字写就绪时，可以将数据写到套接字中
				toiptr += n;			/* # just read */
				FD_SET(sockfd, &wset);	/* try and write to socket below */
			}
		}

		if (FD_ISSET(sockfd, &rset)) {
			//如果套接字读就绪，则将输入写入到fr缓冲区中
			if ( (n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
				//如果发生EWOULDBLOCK错误，则忽略它（这里没有处理该错误的代码）
				//通常情况下这种条件“不应该发生”，因为这意味着，select告知我们
				//相应描述符可读，然而read该描述符却返回EWOULDBLOCK错误

				//如果错误不是EWOULDBLOCK错误，则输出该错误
				if (errno != EWOULDBLOCK)
					err_sys("read error on socket");

			} else if (n == 0) {
#ifdef	VOL2
				fprintf(stderr, "%s: EOF on socket\n", gf_time());
#endif			//如果read返回0，表示服务器发送来了FIN（EOF）
				//如果标准输入上已经遇到EOF，则表示没问题，
				//如果标准输入上没有遇到EOF，表示服务器的EOF并非预期，则输出错误
				if (stdineof)
					return;		/* normal termination */
				else
					err_quit("str_cli: server terminated prematurely");

			} else {
#ifdef	VOL2
				fprintf(stderr, "%s: read %d bytes from socket\n",
								gf_time(), n);
#endif			//正常情况
				//从套接字读入一些数据，相应低增加friptr
				//打开写描述符集中与标准输出对应的位
				//如果标准输出写就绪，可以将数据写到标准输出
				friptr += n;		/* # just read */
				FD_SET(STDOUT_FILENO, &wset);	/* try and write below */
			}
		}
/* end nonb2 */
/* include nonb3 */
		//如果标准输出可写，且要写的字节数大于0，则调用write
		if (FD_ISSET(STDOUT_FILENO, &wset) && ( (n = friptr - froptr) > 0)) {
			if ( (nwritten = write(STDOUT_FILENO, froptr, n)) < 0) {
				//如果发生EWOULDBLOCK错误，则忽略它（这里没有处理该错误的代码）
				//在该条件下，完全有可能发生，因为上述代码“处理套接字可读”时，
				//在不清楚write是否会成功的前提下就打开了写描述符集中与标准输出对应的位

				//如果错误不是EWOULDBLOCK错误，则输出该错误
				if (errno != EWOULDBLOCK)
					err_sys("write error to stdout");

			} else {
#ifdef	VOL2
				fprintf(stderr, "%s: wrote %d bytes to stdout\n",
								gf_time(), nwritten);
#endif			
				//写成功，froptr增加已写出的字节数，
				//如果froptr追上friptr，就同时恢复为指向缓冲区开始处
				froptr += nwritten;		/* # just written */
				if (froptr == friptr)
					froptr = friptr = fr;	/* back to beginning of buffer */
			}
		}

		//如果套接字可写，且要写的字节数大于0，则调用write
		if (FD_ISSET(sockfd, &wset) && ( (n = toiptr - tooptr) > 0)) {
			if ( (nwritten = write(sockfd, tooptr, n)) < 0) {
				//如果发生EWOULDBLOCK错误，则忽略它（这里没有处理该错误的代码）
				//在该条件下，完全有可能发生，因为上述代码“处理标准输入可读”时，
				//在不清楚write是否会成功的前提下就打开了写描述符集中与套接字对应的位

				//如果错误不是EWOULDBLOCK错误，则输出该错误
				if (errno != EWOULDBLOCK)
					err_sys("write error to socket");

			} else {
#ifdef	VOL2
				fprintf(stderr, "%s: wrote %d bytes to socket\n",
								gf_time(), nwritten);
#endif			
				//写成功，tooptr增加已写出的字节数
				//如果tooptr追上toiptr，就同时恢复为指向缓冲区开始处
				//且如果标准输入已经EOF，此时需要写到套接字的数据已经写完，
				//则向服务器发送FIN
				tooptr += nwritten;	/* # just written */
				if (tooptr == toiptr) {
					toiptr = tooptr = to;	/* back to beginning of buffer */
					if (stdineof)
						Shutdown(sockfd, SHUT_WR);	/* send FIN */
				}
			}
		}
	}
}
/* end nonb3 */
