/* include tcp_listen */
#include	"unp.h"

int
tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	//套接字将用于被动打开，因为本函数供服务器使用
	hints.ai_flags = AI_PASSIVE;
	//指定地址族为AF_UNSPEC
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//如果不指定主机名host（对于想绑定通配地址的服务器通常如此）
	//AI_PASSIVE和AF_UNSPEC这两个暗示信息将会返回两个套接字地址结构
	//第一个是IPv6的，第二个是IPv4的（假定运行在一个双栈主机上）
	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		err_quit("tcp_listen error for %s, %s: %s",
				 host, serv, gai_strerror(n));
	ressave = res;

	//调用socket和bind函数，如果任何一个调用失败，那就忽略当前addrinfo结构而改用下一个
	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one */

		//对TCP服务器总是设置SO_REUSEADDR套接字选项
		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		Close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	//检查是否失败（遍历完整个链表），如果失败则显示一个出错消息并终止
	if (res == NULL)	/* errno from final socket() or bind() */
		err_sys("tcp_listen error for %s, %s", host, serv);

	Listen(listenfd, LISTENQ);

	//通过addrlenp指针返回协议地址的大小
	if (addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(listenfd);
}
/* end tcp_listen */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */

int
Tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(tcp_listen(host, serv, addrlenp));
}
