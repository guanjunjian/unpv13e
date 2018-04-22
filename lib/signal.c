/* include signal */
#include	"unp.h"

//typedef	void	Sigfunc(int);
/*
** @signo:信号名
** @func:指向处置函数的指针、或为常值SIG_IGN、SIG_DEF
** 返回值：signal的返回值为一个Sigfunc类型的函数指针
** 这个指针指向的函数仅有一个整型参数，且没有返回值
*/
Sigfunc *
signal(int signo, Sigfunc *func)
{
	//处置结构
	struct sigaction	act, oact;

	//设置处理函数
	act.sa_handler = func;
	/*
	** 作用：设置处理函数的信号掩码
	** POSIX允许指定一组信号，它们在信号处理函数被调用时阻塞。任何阻塞的信号都不能递交给进程
	** 这里是把act.sa_mask设为空集，意味着在该信号处理函数运行期间，不阻塞额外的信号
	** 注意：这里的阻塞与之前说的阻塞于Accept函数的意义不一样
	** 这里的阻塞指：阻塞某个信号或信号集，防止它们在阻塞期间递交。反操作为：解阻塞
	** 之前的阻塞指：阻塞在某个系统调用上，这个系统调用因没有必要资源而必须等待，直到这些资源可用才返回。等待期间进程进入睡眠状态。相对的概念为：非阻塞
	*/
	sigemptyset(&act.sa_mask);
	//设置SA_RESTART标志，可选的。如果设置，由相应信号中断的系统调用将由内核自动重启
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	//调用sigaction函数，并将相应信号的旧行为作为signal函数的返回值
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}
