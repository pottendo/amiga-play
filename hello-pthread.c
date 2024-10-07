#include <proto/exec.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <proto/dos.h>

pthread_t t;
pthread_mutex_t m;
sem_t sem;

void *thread_fun(void *arg)
{
    int it = 0;
    printf("%s: started, waiting for mutex...\n", __FUNCTION__);
    pthread_mutex_lock(&m);
    printf("%s: started, waiting for semaphore...\n", __FUNCTION__);
    sem_wait(&sem);
    printf("%s: yes, let's go...\n", __FUNCTION__);
    while (1)
    {
	printf("%s: %d\n", __FUNCTION__, it++);
	Delay(100);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int ret;
    printf("Hello pthread world\n");

    ret = pthread_mutex_init(&m, NULL);
    if (ret < 0)
	return -1;
    ret = sem_init(&sem, 0, 0);
    if (ret < 0)
	return -1;
    pthread_mutex_lock(&m);
    ret = pthread_create(&t, NULL, thread_fun, NULL);
    if (ret < 0)
    {
	perror("pthread_create failed.");
	return -1;
    }
    ret = pthread_detach(t);
    if (ret != 0)
    {
	perror("pthread_detach failed.");
	printf("errno = %d, ret = %d, EINVAL=%d, ESRCH=%d\n", errno, ret, EINVAL, ESRCH);
    }

    Delay(150);
    printf("%s: unlocking mutex\n", __FUNCTION__);
    pthread_mutex_unlock(&m);
    Delay(150);
    printf("%s: unlocking sem\n", __FUNCTION__);
    sem_post(&sem);
    
    int it = 0;
    while (1)
    {
	Delay(150);
	printf("%s: it = %d\n", __FUNCTION__, it++);
    }
    return 0;
}
