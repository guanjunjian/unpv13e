/* include my_lock_init */
#include	"unpthread.h"
#include	<sys/mman.h>

static pthread_mutex_t	*mptr;	/* actual mutex will be in shared memory */

void
my_lock_init(char *pathname)
{
	int		fd;
	pthread_mutexattr_t	mattr;

	fd = Open("/dev/zero", O_RDWR, 0);

	mptr = Mmap(0, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
	Close(fd);

	//以下三步的作用：
	//调用一些Pthread库函数，告诉函数库：
	//这是一个位于共享内存区中的互斥锁
	//将用于不同进程之间的上锁
	//1.为互斥锁以默认属性初始化一个phtread_mutexattr_t结构
	Pthread_mutexattr_init(&mattr);
	//2.赋予该结构PTHREAD_PROCESS_SHARED属性
	//该属性的默认值为PTHREAD_PROCESS_PRIVATE，即只允许单个进程内使用
	Pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	//3.以mattr中的属性初始化mptr（共享内存区中的互斥锁）
	Pthread_mutex_init(mptr, &mattr);
}
/* end my_lock_init */

/* include my_lock_wait */
void
my_lock_wait()
{
	Pthread_mutex_lock(mptr);
}

void
my_lock_release()
{
	Pthread_mutex_unlock(mptr);
}
/* end my_lock_wait */
