#define _GNU_SOURCE
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sched.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<sys/wait.h>
#include<signal.h>
#define TURE 1


typedef struct PROGRAM{
	char name[10];
	pid_t pid;
	int ready;
	int execute;
}Program;
Program program[20];

static int time, running, numfin, last_switch, numpro;
char type[10];

void unit_time(){
    volatile unsigned long i;
    for(i = 0; i < 1000000UL; i++);
}

int compare(const void *A, const void *B){
	Program a = *(Program *)A;
	Program b = *(Program *)B;
	return (a.ready-b.ready);
}

void cpu_assign(int pid, int core){
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core, &mask);
    
    if(sched_setaffinity(pid, sizeof(mask), &mask) < 0){
        printf("sched_setaffinity fault\n");
        exit(1);    
    }

    return;
}

int pro_execute(Program program){
    int pid = fork();

    if(pid < 0){
        printf("fork error\n");
        return -1;
    }

    if(pid == 0){
        unsigned long start_s, start_ns, end_s, end_ns;
        char string[200];
        syscall(334, &start_s, &start_ns);
        for(int i = 0; i < program.execute; i++){
            unit_time();
        }
            syscall(334, &end_s, &end_ns);
            sprintf(string, "[Project1] %d %lu.%09lu %lu.%09lu\n", getpid(), start_s, start_ns, end_s, end_ns);
            syscall(333, string);
            exit(0);
    }
    cpu_assign(pid, 1);
    return pid;
}

int demote(int pid){
    struct sched_param param;

    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_IDLE, &param);

    return ret;
}

int promote(int pid){
    struct sched_param param;

    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_OTHER, &param);

    return ret;
}

int next_program(){
    if(running != -1 && (type[0] == 'S' || type[0] == 'F'))
        return running;

    int ret = -1;

    if(type[0] == 'P' || type[0] == 'S'){
        for(int i = 0; i < numpro; i++){
            if(program[i].pid == -1 || program[i].execute == 0)
                continue;
            if(ret == -1 || program[i].execute < program[ret].execute)
                ret = i;
        }
    }else if(type[0] == 'F'){
        for(int i = 0; i < numpro; i++){
            if(program[i].pid == -1 || program[i].execute == 0)
                continue;
            if(ret == -1 || program[i].ready < program[ret].ready)
                ret = i;
        }
    }else if(type[0] == 'R'){
        if(running == -1){
            for(int i = 0; i < numpro; i++){
                if(program[i].pid != -1 && program[i].execute > 0){
                    ret = i;
                    break;
                }
            }
        }else if((time - last_switch) % 500 == 0){
            ret = (running + 1) % numpro;
            while(program[ret].pid == -1 || program[ret].execute == 0)
                ret = (ret + 1) % numpro;

        }else{
            ret = running;
        }
    }
    return ret;
}

int scheduling(){
	cpu_assign(getpid(), 0);
	promote(getpid());
			
	time = 0;
	running = -1;
	numfin = 0;

	while(TURE){
        
        if(running != -1 && program[running].execute == 0){
            waitpid(program[running].pid, NULL, 0);
	    printf("%s %d\n", program[running].name, program[running].pid);
            running = -1;
            numfin++;
            
            if(numfin == numpro)
                break;
        }
        
        for(int i = 0; i < numpro; i++){
            if(program[i].ready == time){
                program[i].pid = pro_execute(program[i]);
                demote(program[i].pid);
            }
        }
        
        int next = next_program();
        if(next != -1){
            if(running != next){
                promote(program[next].pid);
                demote(program[running].pid);
                running = next;
                last_switch = time;
            }
        }
        
        unit_time();
        if(running != -1)
            program[running].execute--;
	    time++;
    }


}

int main(){
	scanf("%s%d", type, &numpro);

	for(int i = 0; i < numpro; i++){
		scanf("%s%d%d", program[i].name, &program[i].ready, &program[i].execute);
		program[i].pid = -1;
	}
	qsort(program, numpro, sizeof(Program), compare);

	scheduling();	
	return 0;
}
