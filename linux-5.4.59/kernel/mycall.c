#include <linux/syscalls.h>
#include <linux/kernel.h>
#include<linux/sched/task.h>

SYSCALL_DEFINE0(mycall){
	printk("here is mycall\n");
	return 11;
}

int mycall(struct prinfo *buf, int *nr){
	read_lock(&tasklist_lock);
	//
	
	read_unlock(&tasklist_loc);
}
