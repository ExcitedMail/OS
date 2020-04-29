#include<linux/kernel.h>
#include<linux/linkage.h>
#include<linux/ktime.h>
#include<linux/timekeeping.h>

asmlinkage int sys_my_timestamp(unsigned long *sec, unsigned long *nsec){
	struct timespec time;
	getnstimeofday(&time);
	*sec = time.tv_sec;
	*nsec = time.tv_nsec;
	return 0;
}