#include	"unp.h"

int
main(int argc, char **argv)
{
	int		fd;

	if (argc != 4)
		err_quit("openfile <sockfd#> <filename> <mode>");

	//命令行参数中的打开方式两个由my_open格式化为字符串，
	//需要使用atoi把它们转回整数
	//argv[2]为待打开文件的路径名；argv[3]为打开方式
	if ( (fd = open(argv[2], atoi(argv[3]))) < 0)
		//如果出错，与open对应的errno值就作为进程退出状态的返回
		exit( (errno > 0) ? errno : 255 );

	//argv[1]为流管道的一端对应的描述符
	//将描述符传递回父进程
	if (write_fd(atoi(argv[1]), "", 1, fd) < 0)
		exit( (errno > 0) ? errno : 255 );

	//write_fd把描述符传递回父进程之后，本进程立即终止
	//本章之前说过：发送进程可以不等落地就关闭已传递的描述符（调用exit时发生）
	//因为内核知道该描述符在飞行中，从而为接收进程保持其打开状态
	exit(0);
}
