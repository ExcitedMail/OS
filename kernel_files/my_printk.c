#include<linux/kernel.h>
#include<linux/linkage.h>

asmlinkage int sys_my_printk(char *buf){
	printk("%s\n", buf);
	return 0;
}