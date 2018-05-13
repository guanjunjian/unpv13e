/* include web1 */
#include	"web.h"

int
main(int argc, char **argv)
{
	int		i, fd, n, maxnconn, flags, error;
	char	buf[MAXLINE];
	fd_set	rs, ws;

	if (argc < 5)
		err_quit("usage: web <#conns> <hostname> <homepage> <file1> ...");
	//获取最多执行连接数
	maxnconn = atoi(argv[1]);

	//根据命令行参数的相关信息填写file结构数组
	nfiles = min(argc - 4, MAXFILES);
	for (i = 0; i < nfiles; i++) {
		file[i].f_name = argv[i + 4];
		file[i].f_host = argv[2];
		file[i].f_flags = 0;
	}
	printf("nfiles = %d\n", nfiles);

	//创建一个TCP连接，发出一个命令到服务器，读取主页
	//第一个连接，需要在我们开始并行建立多个连接之前独自完成
	home_page(argv[2], argv[3]);

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	maxfd = -1;
	//nlefttoread是待读取的文件数（当它达到0时程序任务完成）
	//nlefttoconn是尚无TCP连接的文件数
	//nconn是当前打开着的连接数（它不能超过命令行参数[1]）
	nlefttoread = nlefttoconn = nfiles;
	nconn = 0;
/* end web1 */
/* include web2 */
	//主循环：
	//只要还有文件要处理（nlefttoread大于0）
	while (nlefttoread > 0) {
		//如果没有达到最大并行连接数且另有连接需要建立
		//那就找到一个尚未处理的文件（由值为0的f_flags指示）
		//然后调用start_connect发起另一连接
		while (nconn < maxnconn && nlefttoconn > 0) {
				/* 4find a file to read */
			for (i = 0 ; i < nfiles; i++)
				if (file[i].f_flags == 0)
					break;
			if (i == nfiles)
				err_quit("nlefttoconn = %d but nothing found", nlefttoconn);
			//发起非阻塞connect
			start_connect(&file[i]);
			nconn++;
			nlefttoconn--;
		}

		rs = rset;
		ws = wset;
		//在所有活跃的描述符上使用select，
		//以便处理非阻塞连接的建立，又处理来自服务器的数据
		//一个非阻塞connect正在进展的描述符可能会同时开启读写
		//连接建立完毕并正在等待来自服务器的数据的描述符只会开启读
		n = Select(maxfd+1, &rs, &ws, NULL, NULL);

		//遍历files结构数组中的元素，确定哪些描述符需要处理
		for (i = 0; i < nfiles; i++) {
			flags = file[i].f_flags;
			//flags为0表示尚未开始处理
			//flags为F_DONE表示处理完成
			//对于这两种flags直接跳过
			if (flags == 0 || flags & F_DONE)
				continue;
			fd = file[i].f_fd;
			//对于设置了F_CONNECTING标志的一个描述符
			//如果它在读描述符集或写描述符集中对应的位已经打开
			//那么非阻塞connect已经完成
			//调用getsockopt获取该套接字的待处理错误，如果为0，那么连接成功
			if (flags & F_CONNECTING &&
				(FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws))) {
				n = sizeof(error);
				if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &n) < 0 ||
					error != 0) {
					err_ret("nonblocking connect failed for %s",
							file[i].f_name);
				}
					/* 4connection established */
				printf("connection established for %s\n", file[i].f_name);
				//connect成功的话，关闭该描述符在写描述符集中对应的位
				//并向服务器发送HTTP请求到服务器
				FD_CLR(fd, &wset);		/* no more writeability test */
				write_get_cmd(&file[i]);/* write() the GET command */

			} else if (flags & F_READING && FD_ISSET(fd, &rs)) {
				//对于设置了F_READING的描述符，且它读就绪
				//调用read获取数据
				if ( (n = Read(fd, buf, sizeof(buf))) == 0) {
					//如果read返回0，表示收到服务器的FIN
					//关闭套接字，并设置F_DONE标志
					//关闭该套接字在读描述符集中对应的位
					printf("end-of-file on %s\n", file[i].f_name);
					Close(fd);
					file[i].f_flags = F_DONE;	/* clears F_READING */
					FD_CLR(fd, &rset);
					nconn--;
					nlefttoread--;
				} else {
					printf("read %d bytes from %s\n", n, file[i].f_name);
				}
			}
		}
	}
	exit(0);
}
/* end web2 */
