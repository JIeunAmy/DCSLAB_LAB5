#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
struct prinfo{
	int64_t state;
	pid_t pid;
	pid_t parent_pid;
	pid_t first_child_pid;
	pid_t next_sibling_pid;
	int64_t uid;
	char comm[64];
	int depth;
};

int check_num();
void print_tree();

int main(int argc, char *argv[]){
	struct prinfo *p;
	int nr = 10;
	int result;
	int rc;
	if(argc>1){
		if(check_num(argv[1])){
			nr = atoi(argv[1]);
		}
		else{
			printf("numbers only!\n");
			exit(-1);
		}	
	}
	p = calloc(nr, sizeof(struct prinfo));
	if(p==NULL){
		printf("insert number of process you want to see. larger than 0. e.g) ./test.out 10");
		exit(-1);
	}
	
	rc = syscall(548,p, &nr);
	print_tree(p,nr);	
	
	return 0;
}
int check_num(char *num){
        for (int i=0;num[i]!='\0';++i){
		if(num[i]<'0'|| num[i]>'9')
			return 0;
	}
	return 1;
}

void print_tree(struct prinfo *p, int nr){
	for(int i=0;i<nr;i++){
		int dp = 0;
		while(dp++<p[i].depth)
			printf("  ");	
		
		printf("%s,%d,%lld,%d,%d,%d,%lld\n", p[i].comm, p[i].pid, p[i].state, 
		p[i].parent_pid, p[i].first_child_pid, p[i].next_sibling_pid, p[i].uid);
	}
}
