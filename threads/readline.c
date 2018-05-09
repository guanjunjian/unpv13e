/* include readline1 */
#include	"unpthread.h"

//存储readline函数使用的键
static pthread_key_t	rl_key;
//用以判断Pthread_key_create是否第一次调用
static pthread_once_t	rl_once = PTHREAD_ONCE_INIT;

static void
readline_destructor(void *ptr)
{
	free(ptr);
}

//由ptherad_once调用，创建readline使用的键
static void
readline_once(void)
{
	Pthread_key_create(&rl_key, readline_destructor);
}

//存储线程特定数据，旧版本中声明为static
//调用readline的每个线程都由readline动态分配一个Rline结构，
//然后由readline_destructor函数析构
typedef struct {
  int	 rl_cnt;			/* initialize to 0 */
  char	*rl_bufptr;			/* initialize to rl_buf */
  char	 rl_buf[MAXLINE];
} Rline;
/* end readline1 */

/* include readline2 */
/**
** @tsd：指向预先为本线程分配的Rline结构（线程特定数据）的一个指针
*/
static ssize_t
my_read(Rline *tsd, int fd, char *ptr)
{
	if (tsd->rl_cnt <= 0) {
again:
		if ( (tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
			if (errno == EINTR)
				goto again;
			return(-1);
		} else if (tsd->rl_cnt == 0)
			return(0);
		tsd->rl_bufptr = tsd->rl_buf;
	}

	tsd->rl_cnt--;
	*ptr = *tsd->rl_bufptr++;
	return(1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
	size_t		n, rc;
	char	c, *ptr;
	Rline	*tsd;
	//调用pthread_once使得本进程内第一个调用readline的线程通过调用
	//Pthread_key_create创建线程特定数据使用的键
	Pthread_once(&rl_once, readline_once);
	//pthread_getspecific返回指向特定于本线程的Rline结构的指针
	//如果是本线程首次调用，返回值为NULL
	//首次调用则分配一个Rline结构的空间，并由calloc将其rl_cnt成员初始化为0
	//再调用Pthread_setspecific为本线程存储这个指针
	//一下次本线程调用readline时，pthread_getspecific可以返回一个非空指针（指向本线程的线程特定数据）
	if ( (tsd = pthread_getspecific(rl_key)) == NULL) {
		tsd = Calloc(1, sizeof(Rline));		/* init to 0 */
		Pthread_setspecific(rl_key, tsd);
	}

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ( (rc = my_read(tsd, fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n')
				break;
		} else if (rc == 0) {
			*ptr = 0;
			return(n - 1);		/* EOF, n - 1 bytes read */
		} else
			return(-1);		/* error, errno set by read() */
	}

	*ptr = 0;
	return(n);
}
/* end readline2 */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
	ssize_t		n;

	if ( (n = readline(fd, ptr, maxlen)) < 0)
		err_sys("readline error");
	return(n);
}
