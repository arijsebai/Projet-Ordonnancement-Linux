#ifndef PROCESS_H
#define PROCESS_H

#define NAME_LEN 64
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define ZOMBIE 3


struct process {
    char name[NAME_LEN];
    int arrival_time;    
    int exec_time;   
    int priority;   
    int remaining_time;  
    int waiting_time;
    int status;
    int end_time;
    int wait_time;   
};

#endif 
