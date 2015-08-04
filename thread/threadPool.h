#include <stdio.h>   
#include <stdlib.h>   
#include <sys/types.h>   
#include <pthread.h>   
#include <signal.h>   

#ifndef TPBOOL   
typedef int TPBOOL;  
#endif   

#ifndef TRUE   
#define TRUE 1   
#endif   

#ifndef FALSE   
#define FALSE 0   
#endif   

#define BUSY_THRESHOLD 0.5  //(busy thread)/(all thread threshold)   
#define MANAGE_INTERVAL 5   //tp manage thread sleep interval   


//thread parm   
struct tp_work_desc{  
    char *inum; //call in   
    char *onum; //call out   
    int chnum;  //channel num   
};  

//base thread struct   
struct tp_work{  
    //main process function. user interface   
    void (*process_job)(struct tp_work *work, struct tp_work_desc *job);  
};  

//thread info   
struct tp_thread_info{  
    pthread_t       thread_id;  //thread id num   
    TPBOOL          is_busy;    //thread status:true-busy;flase-idle   
    pthread_cond_t          thread_cond;      
    pthread_mutex_t     thread_lock;  
    struct tp_work         *th_work;  
    struct tp_work_desc        *th_job;  
};  

/*main thread pool struct  */ 
struct tp_thread_pool{  
    TPBOOL (*init)(struct tp_thread_pool *tp);  
    void (*close)(struct tp_thread_pool *tp);  
    void (*process_job)(struct tp_thread_pool *tp, struct tp_work *worker, struct tp_work_desc *job);  
    int  (*get_thread_by_id)(struct tp_thread_pool *tp, int id);  
    TPBOOL (*add_thread)(struct tp_thread_pool *tp);  
    TPBOOL (*delete_thread)(struct tp_thread_pool *tp);  
    int (*get_tp_status)(struct tp_thread_pool *tp);  

    int min_th_num;     //min thread number in the pool   
    int cur_th_num;     //current thread number in the pool   
    int max_th_num;         //max thread number in the pool   
    pthread_mutex_t tp_lock;  
    pthread_t manage_thread_id; //manage thread id num   
    struct tp_thread_info *thread_info;    //work thread relative thread info   
};  

struct tp_thread_pool *creat_thread_pool(int min_num, int max_num); 

