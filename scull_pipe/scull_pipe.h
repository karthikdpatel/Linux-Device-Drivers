struct scull_pipe_dev
{
	int BUFFER_SIZE;
	char *buffer_end, *buffer_start;
	char *rp, *wp;
	wait_queue_head_t inq, outq;
	struct semaphore sem;
	struct cdev cdev;
};

