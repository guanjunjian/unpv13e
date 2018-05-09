typedef struct {
  pthread_t		thread_tid;		/* thread ID */
  long			thread_count;	/* # connections handled */
} Thread;
Thread	*tptr;		/* array of Thread structures; calloc'ed */

#define	MAXNCLI	32
//由主线程往其中存入已接受的已连接套接字描述符
//由线程池中的线程从中取出一个以服务相应的客户
//iput是主线程将往该数组中存入的下一个元素的下标
//iget是线程池中某个线程从该数组中取出的下一个元素的下标
int					clifd[MAXNCLI], iget, iput;
//互斥量，用户clifd的互斥访问
pthread_mutex_t		clifd_mutex;
//条件变量，用以决定是否还有需要处理的连接
pthread_cond_t		clifd_cond;
