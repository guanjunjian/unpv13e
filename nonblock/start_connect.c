#include	"web.h"

void
start_connect(struct file *fptr)
{
	int				fd, flags, n;
	struct addrinfo	*ai;

	//Host_serv查找并转换主机名和服务名
	//它返回指向某个addrinfo结构数组的一个指针
	//只使用其中第一个结构
	ai = Host_serv(fptr->f_host, SERV, 0, SOCK_STREAM);

	//创建一个TCP套接字
	fd = Socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	fptr->f_fd = fd;
	printf("start_connect for %s, fd %d\n", fptr->f_name, fd);

		/* 4Set socket nonblocking */
	//把该套接字设置为非阻塞
	flags = Fcntl(fd, F_GETFL, 0);
	Fcntl(fd, F_SETFL, flags | O_NONBLOCK);

		/* 4Initiate nonblocking connect to the server. */
	//发起非阻塞connect
	if ( (n = connect(fd, ai->ai_addr, ai->ai_addrlen)) < 0) {
		if (errno != EINPROGRESS)
			err_sys("nonblocking connect error");
		//如果没有立马连接成功
		//把相应文件的标志设为“正在连接”F_CONNECTING
		fptr->f_flags = F_CONNECTING;
		//设置该套接字在读描述符集合写描述符集中对应的位
		//因为select将等待其中任何一个条件变为真，作为连接已建立完毕的指示
		FD_SET(fd, &rset);			/* select for reading and writing */
		FD_SET(fd, &wset);
		if (fd > maxfd)
			maxfd = fd;

	} else if (n >= 0)				/* connect is already done */
		//如果connect立即成功返回
		//表示连接已经建立
		//调用write_get_cmd函数发送一个命令到服务器
		write_get_cmd(fptr);	/* write() the GET command */

	//connet把套接字设为非阻塞后，不再把它重置为默认的阻塞模式
	//这么做没有问题，因为我们只往套接字中写入少量的数据，
	//写入的数据比发送缓冲区小很多，因此可是说是不会阻塞的
}
