#define SCULL_QSET 2
#define SCULL_QUANTUM 5

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;
	int quantum;
	int qset;
	unsigned long size;
	unsigned int access_key;
//	struct semaphore sem;
	struct cdev cdev;
};
