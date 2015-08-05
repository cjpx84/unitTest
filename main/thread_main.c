#include<stdio.h>
#include <thread.h>



void * thread_func1(void *arg)
{
  printf("thread 1 run\n");

  while(1)
  {
   sleep(100);
}
 return NULL;
}


void * thread_func2(void *arg)
{
  printf("thread 2 run\n");

sleep(200);
  while(1)
  {
}
 return NULL;
}

void test_func2(int x,int y)
{
  printf("test func 2 %d\n",x+y);
  
 printf("x+y=%d\n",x+y);
  
printf("test func 2 end\n");
}
void test_func(void)
{
  printf("test func\n");
 test_func2(3,5);
}
void * thread_func3(void *arg)
{
  int task= *((int *)arg);
  printf("thread 3 run\n");

  printf("task:%d\n",task);
   sleep(100);
  test_func();
 return NULL;
}

int main()
{
 int task = 100;
  thread_list_head_init();
  thread_create("thread1",70,0,0,thread_func2,NULL);
  thread_create("thread2",60,0,0,thread_func1,NULL);
  thread_create("DEADLOOPTEST",50,0,0,thread_func3,&task);
  show_all_thread();

while(1){
 show_all_thread();
 sleep(10);
}

  return 0;
}
