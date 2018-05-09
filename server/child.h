typedef struct {
  pid_t		child_pid;		/* 子进程ID */
  int		child_pipefd;	/* 父进程连接到该子进程的字节流管道描述符 */
  int		child_status;	/* 子进程状态，0 = ready */
  long		child_count;	/* # 子进程已处理客户的计数 */
} Child;

Child	*cptr;		/* array of Child structures; calloc'ed */
