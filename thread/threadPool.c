#include "thread-pool.h"   

static void *tp_work_thread(void *pthread);  
static void *tp_manage_thread(void *pthread);  

static TPBOOL tp_init(struct tp_thread_pool *tp);  
static void tp_close(struct tp_thread_pool *tp);  
static void tp_process_job(struct tp_thread_pool *tp, struct tp_work *worker, struct tp_work_desc *job);  
static int  tp_get_thread_by_id(struct tp_thread_pool *tp, int id);  
static TPBOOL tp_add_thread(struct tp_thread_pool *tp);  
static TPBOOL tp_delete_thread(struct tp_thread_pool *tp);  
static int  tp_get_tp_status(struct tp_thread_pool *tp);  

/** 
  * user interface. creat thread pool. 
  * para: 
  *     min_num: min thread number to be created in the pool 
  *     max_num: max thread number to be created in the pool
  * return: 
  *     thread pool struct instance be created successfully 
  */  
struct tp_thread_pool *creat_thread_pool(int min_num, int max_num){  
    struct tp_thread_pool *tp;  
    tp = (struct tp_thread_pool*)malloc(sizeof(struct tp_thread_pool));   

    memset(tp, 0, sizeof(struct tp_thread_pool));  

       
    tp->init = tp_init;  
    tp->close = tp_close;  
    tp->process_job = tp_process_job;  
    tp->get_thread_by_id = tp_get_thread_by_id;  
    tp->add_thread = tp_add_thread;  
    tp->delete_thread = tp_delete_thread;  
    tp->get_tp_status = tp_get_tp_status;  

      
    tp->min_th_num = min_num;  
    tp->cur_th_num = this->min_th_num;  
    tp->max_th_num = max_num;  
    pthread_mutex_init(&tp->tp_lock, NULL);  

    //malloc mem for num thread info struct   
    if(NULL != tp->thread_info)  
        free(tp->thread_info); 
	/*alloc work thread information,we may creat max_num work threads*/
	/*so must alloc max_num*/
    tp->thread_info = (struct tp_thread_info*)malloc(sizeof(struct tp_thread_info)*tp->max_th_num);  

    return tp;  
}  

  
/** 
  * member function reality. thread pool init function. 
  * para: 
  *     poll: thread pool struct instance pointer 
  * return: 
  *     true: successful; false: failed 
  */  
TPBOOL tp_init(struct tp_thread_pool *pool){  
    int i;  
    int err;  

    /*creat work thread and init work thread info*/   
    for(i=0;i<pool->min_th_num;i++){  
        pthread_cond_init(&pool->thread_info[i].thread_cond, NULL);  
        pthread_mutex_init(&pool->thread_info[i].thread_lock, NULL);  

        err = pthread_create(&pool->thread_info[i].thread_id, NULL, tp_work_thread, pool);  
        if(0 != err){  
            printf("tp_init: creat work thread failed\n");  
            return FALSE;  
        }  
        printf("tp_init: creat work thread %d\n", pool->thread_info[i].thread_id);  
    }  

    /*creat manage thread */ 
    err = pthread_create(&pool->manage_thread_id, NULL, tp_manage_thread, pool);  
    if(0 != err){  
        printf("tp_init: creat manage thread failed\n");  
        return FALSE;  
    }  
    printf("tp_init: creat manage thread %d\n", pool->manage_thread_id);  

    return TRUE;  
}  

/** 
  * member function reality. thread pool entirely close function. 
  * para: 
  *     pool: thread pool struct instance pointer 
  * return: 
  */  
void tp_close(tp_thread_pool *pool){  
    int i;  

    /*close work thread */  
    for(i=0;i<pool->cur_th_num;i++){  
        kill(pool->thread_info[i].thread_id, SIGKILL);  
        pthread_mutex_destroy(&pool->thread_info[i].thread_lock);  
        pthread_cond_destroy(&pool->thread_info[i].thread_cond);  
        printf("tp_close: kill work thread %d\n", pool->thread_info[i].thread_id);  
    }  

    //close manage thread   
    kill(pool->manage_thread_id, SIGKILL);  
    pthread_mutex_destroy(&this->tp_lock);  
    printf("tp_close: kill manage thread %d\n", pool->manage_thread_id);  

    /*free thread struct */  
    free(pool->thread_info);  
}  

/**  
  * after getting own worker and job, user may use the function to process the task. 
  * para: 
  *     this: thread pool struct instance ponter 
  * worker: user task reality. 
  * job: user task para 
  * return: 
  */  
void tp_process_job(struct tp_thread_pool *pool, struct tp_work *worker, struct tp_work_desc *job){  
    int i;  
    int tmpid;  

    /*fill pool->thread_info's relative work key */  
    for(i=0;i<pool->cur_th_num;i++){  
        pthread_mutex_lock(&pool->thread_info[i].thread_lock);  
        if(!pool->thread_info[i].is_busy){  
            printf("tp_process_job: %d thread idle, thread id is %d\n", i, pool->thread_info[i].thread_id);  
            /*thread state be set busy before work */  
            pool->thread_info[i].is_busy = TRUE;  
            pthread_mutex_unlock(&pool->thread_info[i].thread_lock);  

            pool->thread_info[i].th_work = worker;  
            pool->thread_info[i].th_job = job;  

            printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, pool->thread_info[i].thread_id);  
            pthread_cond_signal(&pool->thread_info[i].thread_cond);  

            return;  
        }  
        else   
            pthread_mutex_unlock(&pool->thread_info[i].thread_lock);       
    }//end of for   

    //if all current thread are busy, new thread is created here   
    pthread_mutex_lock(&pool->tp_lock);  
    if( pool->add_thread(pool) ){  
        i = pool->cur_th_num - 1;  
        tmpid = pool->thread_info[i].thread_id;  
        pool->thread_info[i].th_work = worker;  
        pool->thread_info[i].th_job = job;  
    }  
    pthread_mutex_unlock(&pool->tp_lock);  

    //send cond to work thread   
    printf("tp_process_job: informing idle working thread %d, thread id is %d\n", i, pool->thread_info[i].thread_id);  
    pthread_cond_signal(&pool->thread_info[i].thread_cond);  
    return;   
}  

  
int tp_get_thread_by_id(struct tp_thread_pool *pool, int id){  
    int i;  

    for(i=0;i<pool->cur_th_num;i++){  
        if(id == pool->thread_info[i].thread_id)  
            return i;  
    }  

    return -1;  
}  

/** 
  *   add work thread to pool
*/  
static TPBOOL tp_add_thread(struct tp_thread_pool *pool){  
    int err;  
    struct tp_thread_info *new_thread;  

    if( pool->max_th_num <= pool->cur_th_num )  
        return FALSE;  

    //malloc new thread info struct   
    new_thread = &pool->thread_info[pool->cur_th_num];  

    //init new thread's cond & mutex   
    pthread_cond_init(&new_thread->thread_cond, NULL);  
    pthread_mutex_init(&new_thread->thread_lock, NULL);  

    //init status is busy   
    new_thread->is_busy = TRUE;  

    //add current thread number in the pool.   
    pool->cur_th_num++;  

    err = pthread_create(&new_thread->thread_id, NULL, tp_work_thread, pool);  
    if(0 != err){  
        free(new_thread);  
        return FALSE;  
    }  
    printf("tp_add_thread: creat work thread %d\n", pool->thread_info[pool->cur_th_num-1].thread_id);  

    return TRUE;  
}  

/** 
  * delete work thread from pool 
*/  
static TPBOOL tp_delete_thread(struct tp_thread_pool *pool)
{  
	/*current thread num can't < min thread num */  
    if(pool->cur_th_num <= pool->min_th_num) return FALSE;  

    /*if last thread is busy, do nothing*/   
    if(pool->thread_info[pool->cur_th_num-1].is_busy) return FALSE;  

    /*kill the idle thread and free info struct */  
    kill(pool->thread_info[pool->cur_th_num-1].thread_id, SIGKILL);  
    pthread_mutex_destroy(&pool->thread_info[pool->cur_th_num-1].thread_lock);  
    pthread_cond_destroy(&pool->thread_info[pool->cur_th_num-1].thread_cond);  

    /*after deleting idle thread, current thread num -1 */  
    pool->cur_th_num--;  

    return TRUE;  
}  

 
static int  tp_get_tp_status(struct tp_thread_pool *pool){  
    float busy_num = 0.0;  
    int i;  

    //get busy thread number   
    for(i=0;i<pool->cur_th_num;i++){  
        if(pool->thread_info[i].is_busy)  
            busy_num++;  
    }  

    //0.2? or other num?   
    if(busy_num/(pool->cur_th_num) < BUSY_THRESHOLD)  
        return 0;//idle status   
    else  
        return 1;//busy or normal status       
}  

/** 
    work thread entry
  */  
static void *tp_work_thread(void *args)
{  
    /*current thread id */
    pthread_t curid;
	//current thread index in the this->thread_info array
    int idx;   
    struct tp_thread_pool *tp = (tp_thread_pool*)args;   

    /*get current thread id */  
    curid = pthread_self();  

    /*get current thread's idx in the thread info struct array. */  
    idx = tp->get_thread_by_id(tp, curid);  
    if(idx< 0)  
        return;  
    printf("entering working thread %d, thread id is %d\n", idx, curid);  

    /*wait cond for processing real job  */ 
    while( 1 ){  
        pthread_mutex_lock(&tp->thread_info[idx].thread_lock);  
        pthread_cond_wait(&tp->thread_info[idx].thread_cond, &tp->thread_info[idx].thread_lock);  
        pthread_mutex_unlock(&tp->thread_info[idx].thread_lock);        

        printf("%d thread do work!\n", pthread_self());  

        struct tp_work *work = tp->thread_info[idx].th_work;  
        struct tp_work_desc *job = this->thread_info[idx].th_job;  

          
        work->process_job(work, job);  

        //thread state be set idle after work   
        pthread_mutex_lock(&tp->thread_info[idx].thread_lock);          
        tp->thread_info[idx].is_busy = FALSE;  
        pthread_mutex_unlock(&tp->thread_info[idx].thread_lock);  

        printf("%d do work over\n", pthread_self());  
    }     
}  

/** 
  
  */  
static void *tp_manage_thread(void *args){  
    tp_thread_pool *tp = (tp_thread_pool*)args;//main thread pool struct instance   

    //1?   
    sleep(MANAGE_INTERVAL);  

    do{  
        if( tp->get_tp_status(tp) == 0 ){  
            do{  
                if( !tp->delete_thread(tp) )  
                    break;  
            }while(TRUE);  
        }//end for if   

        //1?   
        sleep(MANAGE_INTERVAL);  
    }while(TRUE);  
}  

