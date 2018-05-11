#include	"unp.h"

int
my_open(const char *pathname, int mode)
{
	int			fd, sockfd[2], status;
	pid_t		childpid;
	char		c, argsockfd[10], argmode[10];

	//创建一个流管道sockfd[0]和sockfd[1]
	Socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	if ( (childpid = Fork()) == 0) {		/* child process */
		Close(sockfd[0]);
		//sockfd[1]的描述符号格式化输出到argsockfd
		//打开方式mode格式化输出到argmode
		//调用snprintf进行格式化输出时因为exec的参数必须是字符串
		snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
		snprintf(argmode, sizeof(argmode), "%d", mode);
		//调用execl执行openfile程序
		//该函数不会返回，除非它发生错误
		//一旦成功，openfile程序的main函数就开始执行
		execl("./openfile", "openfile", argsockfd, pathname, argmode,
			  (char *) NULL);
		err_sys("execl error");
	}

	/* parent process - wait for the child to terminate */
	Close(sockfd[1]);			/* close the end we don't use */

	//调用waitpid等待子进程终止
	//子进程的终止状态在status中返回
	Waitpid(childpid, &status, 0);
	//检查该程序是否正常终止（也就是说未被某个信号终止）
	if (WIFEXITED(status) == 0)
		err_quit("child did not terminate");
	//若正常终止，调用WEXITSTATUS宏把终止状态换成“退出状态”
	//“退出状态”的取值在0~255之间
	if ( (status = WEXITSTATUS(status)) == 0)
		//调用read_fd函数通过流管道接收描述符
		//除了描述符外，还读取1个字节的数据，但不对数据进行任何处理
		//“还读取1个字节的数据”的原因：通过流管道发送和接收描述符时，总是发送
		//至少1个字节的数据。要是不这么做，接收进程将难以辨别read_fd的返回值为0
		//意味着“没有数据（但可能伴有一个描述符）”还是“文件已结束”
		//Read_fd函数的前3个参数和read函数一样，第4个参数是指向某个整数的指针
		//用以返回收取的描述符
		Read_fd(sockfd[0], &c, 1, &fd);
	else {
		//如果openfile程序在打开所请求文件时碰到一个错误，
		//它将以相应的errno值作为退出状态终止自身
		errno = status;		/* set errno value from child's status */
		fd = -1;
	}

	Close(sockfd[0]);
	return(fd);
}
