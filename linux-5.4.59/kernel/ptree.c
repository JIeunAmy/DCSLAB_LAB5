/*
 * ptree.c
 *
 *  Created on: Sep 27, 2012
 */

#include "ptree.h"


SYSCALL_DEFINE2(ptree, struct prinfo *, buf, int *, nr)
{
	int rv;
	struct prinfo *kbuf;
	int knr;
	int rc;

	kbuf = kcalloc(*nr, sizeof(struct prinfo), GFP_KERNEL);
	if (kbuf == NULL)
		return -ENOMEM;

	copy_from_user(&knr, nr, sizeof(int));
	copy_from_user(kbuf, buf, sizeof(struct prinfo) * *nr);

	acquire_tasklist_lock();
	printk("read_lock\n");
	rv = find_processes(kbuf, &knr);
	release_tasklist_lock();
	printk("read_unlock\n");

	copy_to_user(buf, kbuf, sizeof(struct prinfo) * *nr);
	copy_to_user(nr, &knr, sizeof(int));

	kfree(kbuf);

	return rv;
}

int has_sibling(struct task_struct *task)
{
	struct list_head *head = &task->parent->children;
	if (list_is_last(&task->sibling, head))
		return false;
	else
		return true;
}

struct task_struct *get_next_node(struct task_struct *_cur, int *depth)
{
	struct task_struct *cur = _cur;
	if (has_children(cur)){
		*depth += 1;
		return list_entry(cur->children.next, struct task_struct, sibling);
	}
	while (!has_sibling(cur)) {
		cur = cur->parent;
		*depth -= 1;
		if (cur->pid == 0)
			return NULL;
	}

	return list_entry(cur->sibling.next, struct task_struct, sibling);
}

void process_node(int idx, struct prinfo *buf, struct task_struct *task, int depth)
{
	struct prinfo to_add;
	struct task_struct *next_sibling;
	struct task_struct *first_child;

	to_add.state = task->state;
	to_add.pid = task->pid;
	to_add.parent_pid = task->parent->pid;
	to_add.uid = task_uid(task).val;
	strncpy(to_add.comm, task->comm, MAX_COMM);
	to_add.depth = depth;
	if (has_children(task)) {
		first_child = list_entry(task->children.next, struct task_struct, 
								sibling);
		to_add.first_child_pid = first_child->pid;
	} else
		to_add.first_child_pid = 0;

	if (has_sibling(task)) {
		next_sibling = list_entry(task->sibling.next, struct task_struct,
								sibling);
		to_add.next_sibling_pid = next_sibling->pid;
	} else
		to_add.next_sibling_pid = 0;

	buf[idx] = to_add;
}

int find_processes(struct prinfo *buf, int *nr)
{
	int buf_idx = 0;
	int depth = -1;
	int total_count = 0;
	struct task_struct *cur = get_init_process();
	//cur = list_entry(cur->children.next, sturct task_struct, sibling);
	while (cur != NULL) {
		if (!is_a_process(cur) || cur->pid == 0) {
			cur = get_next_node(cur, &depth);
			continue;
		}
		total_count++;
		if (buf_idx < *nr) {
			process_node(buf_idx, buf, cur, depth);
			buf_idx++;
		}
		cur = get_next_node(cur, &depth);
	}
	*nr = buf_idx;

	return total_count;
}

void acquire_tasklist_lock(void)
{
	read_lock(&tasklist_lock);
}

void release_tasklist_lock(void)
{
	read_unlock(&tasklist_lock);
}

int no_children(struct task_struct *task)
{
	struct list_head *children;
	if (task == NULL)
		return 0;
	children = &task->children;
	if (list_empty(children))
		return true;
	else
		return false;
}

int has_children(struct task_struct *task)
{
	if (task == NULL)
		return 0;

	return !no_children(task);
}

struct task_struct *get_init_process(void)
{
	return &init_task;
}

int is_a_process(struct task_struct *task)
{
	if (thread_group_empty(task))
		return true; /* A process with 0 threads */
	else  {
		if (thread_group_leader(task))
			return true; /* A process with at least 1 thread */
		else
			return false;
	}
	return false;
}

