#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "ThreadPool.h"


int bExit = 0;

void signal_ctrlc(int sig)
{
//	std::cout<<"get signal:"<<sig<<std::endl;
	bExit = 1;
}


//测试未初始化的条件变量直接被销毁的结果：
//正常，即使pthread_cond_t是未初始化,程序也不会崩溃。
void test01()
{
    pthread_cond_t m_queue_not_full; 
    pthread_cond_destroy(&m_queue_not_full);
    printf("hh\n");
}

//测试当一个线程销毁到锁后，另一个线程再上锁解锁是否会崩溃
//结果：正常，不会崩溃。所以在调用create的过程中，立马调用destory应该不会出现问题(假设create返回但部分线程还在创建中，后面可以用代码优化)。
void test02()
{
    pthread_mutex_t m;
    pthread_mutex_init(&m, NULL);

    pthread_mutex_lock(&m);
    pthread_mutex_unlock(&m);//模拟一个线程执行任务后释放锁

    pthread_mutex_lock(&m);//模拟此时主线程刚刚好调用释放线程池函数，拿到锁后释放锁
    pthread_mutex_destroy(&m);

	printf("hhfasdasad\n");
    pthread_mutex_lock(&m);//模拟另一个线程在锁被销毁后，再次上锁解锁。注意：由于锁被释放，所以这里并不会造成死锁，结果可以看到"hh"输出.
    pthread_mutex_unlock(&m);

    printf("hh\n");

}

/* 线程池中的线程，模拟处理业务 */
void *process(void *arg)
{
	printf("thread working on task %d\n ", *(int *)arg);
    sleep(1);
	printf("task %d is end\n", *(int *)arg);

	return NULL;
}

int main(){

	//1 先创建线程池
	threadpool_t *pool = NULL;
	pool = threadpool_create(3, 15, 100);				/*创建线程池*/
	if(NULL == pool){
		printf("threadpool_create failed.\n");
		return -1;
	}								
	printf("pool inited\n");
	sleep(2);


	//2 模拟客户端请求的任务
	//注意添加任务时num数组大小也要改变，否则会出现未知错误
	int num[300], i;
	int r;
	int busyNum, allThrNum;
	srand(time(NULL));
Task:
	r = rand() % 300;//模拟随机任务数
	for (i = 0; i < r; i++) {
		num[i] = i;
		printf("add task %d\n", i);
		threadpool_add(pool, process, (void*)&num[i]);
	}

	printf("添加完一次任务,任务数=%d, 即0 ~ %d\n", r, r - 1);

	if(bExit){
		goto EXIT;
	}

	/*sleep(2);
	goto Task;//模拟线程池一直有任务.测试2个小时左右，稳定且数据正确,若想测试threadpool_destroy，可以注释掉*/
    sleep(15);
	goto Task;//模拟不断往线程池添加任务,若想测试threadpool_destroy，可以注释掉

EXIT:
	busyNum = threadpool_busy_threadnum(pool);
	printf("BusyNum = %d\n", busyNum);
	allThrNum = threadpool_all_threadnum(pool);
	printf("allThrNum = %d\n", allThrNum);

	//3 等子线程完成任务	    
    sleep(10);

	printf("main sleep ok\n");

	//4 销毁线程池
	threadpool_destroy(pool);

	printf("main end\n");

    return 0;
}