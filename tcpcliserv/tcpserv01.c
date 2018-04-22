#include	"unp.h"

int
main(int argc, char **argv)
{
	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;

	//1.创建套接字，目前为未连接套接字，Listen之后变为监听套接字
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	//2.设置地址结构
    //2.1.将地址结构清空
	bzero(&servaddr, sizeof(servaddr));
    //2.2.设置协议族
	servaddr.sin_family      = AF_INET;
    //2.3.绑定通配地址：告知系统：要是系统是多宿主机，我们将接受目的地址为任何本地接口的连接
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//通配地址INADDR_ANY
    //2.3.绑定端口，#define SERV_PORT 9877
	servaddr.sin_port        = htons(SERV_PORT);//端口号SERV_PORT(9877)

	//3.将套接字绑定到地址
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	//4.将未连接套接字转换为监听套接字
	Listen(listenfd, LISTENQ);

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		//5.阻塞在Accept，等待来自客户端的连接
		connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
        
		//6.为每个客户派生一个处理它们的子进程，子进程会关闭监听套接字（引用计数-1），父进程关闭已连接套接字（引用计数-1），继续在监听套接字监听
		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	//关闭监听套接字（引用计数-1）
			//7.子进程调用str_echo接收并回显客户端发来的消息
			str_echo(connfd);	/* process the request */
			exit(0);
		}
		Close(connfd);		//关闭已连接套接字（引用计数-1）
	}
}