#include <stdio.h>                                                                  //include standard header file
#include <stdint.h>
#include <pthread.h>
#include <time.h>                                                                   //To deal with timespecs
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>                                                            
#include "thread_model.h"                                                           //Importing the Header file provided

pthread_mutex_t m[NUM_MUTEXES];                                                      //initalizing variable for mutex
pthread_t threadId[NUM_THREADS];                                                    //Initializing variable for threads i.e threadId 
pthread_barrier_t barrier;                                                           //Initializing variable for barrier
pthread_t keyboard_thread ;
pthread_attr_t tattr;
struct sched_param param;
pthread_mutex_t mp[10];                                                             //Initializing the dummy mutex for pthread_cond

volatile int flag = 1;
pthread_cond_t cv[10];                                                              //Initializing the threads for pthread_cond 

//defining the periodic routine as P_routine
void* P_routine(void* arg) {                                                        //call to the periodic routine
    
    struct timespec next, period;
    struct Tasks *ptr = (struct Tasks *)arg;                                          //passing the arguments of the threads in pointer to access
    period.tv_nsec = (ptr->period)*1000000;
    volatile uint64_t x;

    //printf("Task created %d..\n",ptr->task_num);                            //verifying if the threads were created
    // printf("Waiting at the barrier...\n");                                     verifying if all the threads are waiting at the barrier
    
    pthread_barrier_wait(&barrier);                                                      //creating the barrier
    // printf("We passed the barrier %d \n",ptr->task_num);
    clock_gettime(CLOCK_MONOTONIC, &next);                      
    //printf("%ld \n %ld \n",next.tv_sec,next.tv_nsec);
    while(flag){                                                                    //Task body starts here
        x= ptr->loop_iter[0];                                                       //loading the value of iteration
        while(x >0){                                                                // compute 1
            x--;
        }
        
        pthread_mutex_lock(&m[ptr->mutex_num]);                                     //lock the mutex for the critical compute
        x= ptr->loop_iter[1];                                                       //loading the value of iteration
        while(x>0){                                                                 //Compute 2
            x--;        
        }
        pthread_mutex_unlock(&m[ptr->mutex_num]);                                   //Unlock the mutex

        x= ptr->loop_iter[2];
        while(x>0){                                                                 //Compute 3

            x--;
        }
        
        next.tv_sec = next.tv_sec + ((next.tv_nsec + period.tv_nsec)/1000000000);                  //wait for period
        next.tv_nsec = ((next.tv_nsec + period.tv_nsec)%1000000000) ;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, 0);
    }
    pthread_exit(NULL);
}

void* keyboard(void* arg)                                                               //event function for keyboard interrupt
{                                                              
    int fd;                                                                             //initializing variables
    struct input_event ie;
    fd = open(KEYBOARD_EVENT_DEV, O_RDONLY);                                                  
    // if(fd == -1) {                                                                    //accessing the keyboarad device 
    //         perror("opening device");                                                  //error if unable to open one
    //         exit(EXIT_FAILURE);
    // }
    pthread_barrier_wait(&barrier);                                                       //adding barrier

    while(flag)
    {
        if(read(fd, &ie, sizeof(ie)))                                                    //continous reading of the keyboard thread
        {
            // printf("%d\t%d\t%d\n", ie.type, ie.code, ie.value);
            if((ie.type==1) && (ie.code==11) && (ie.value==0))
            {                                                                               //condition for getting broadcast on key 0
                pthread_cond_broadcast(&cv[0]);                                             
            }
            for(int i=1;i<=9;i++)
            {
                if((ie.type == 1) && (ie.code == (i+ 1)) && (ie.value == 0))
                {                                                                           //condition for getting broadcast for other key events
                    pthread_cond_broadcast(&cv[i]);
                }
            }
        }
    }

    close(fd);
    pthread_exit(NULL);
}

void* A_routine(void* arg){                                                                     //call to Aperiodic thread

    struct Tasks *ptr = (struct Tasks *)arg;                                                    //Loading values of argument in pointer
    //printf("Task created %d..\n",ptr->task_num);
    volatile uint64_t x;        
    //printf("Waiting at the barrier...\n");
    pthread_barrier_wait(&barrier);                                                             //creating barrier for the thread
    //printf("We passed the barrier %d..\n",ptr->task_num);
    pthread_cond_wait(&cv[ptr->event_key],&mp[ptr->event_key]);

    while(flag)
    {                                                                                            //Task body
        //printf("\nSemaphore %d received\n", ptr->event_key);                                 //verification for events
        x= ptr->loop_iter[0];                                                               //Loading value of loop iterating in x
        while(x >0){                                                                        //Compute 1
            x--;
        }

        pthread_mutex_lock(&m[ptr->mutex_num]);                                             //lock the mutex for critical section
        x= ptr->loop_iter[1];                                                           
        while(x>0){                                                                         //Compute 2
            x--;        
        }
        pthread_mutex_unlock(&m[ptr->mutex_num]);                                            //Unlock mutex
        
        x= ptr->loop_iter[2];
        while(x>0){                                                                          //Compute 3
            x--;
        }
        pthread_cond_wait(&cv[ptr->event_key],&mp[ptr->event_key]);                         //waiting for condition
    }

    pthread_exit(NULL);
}

int main()
{   
    pthread_t main_id;
    main_id = pthread_self();
    param.sched_priority = 99;                                                                  //scheduling priority for main 
    pthread_setschedparam(main_id, SCHED_FIFO, &param);

    for(int i=0;i< NUM_MUTEXES;i++){                                                            // initializing a mutex 
        pthread_mutex_init(&m[i],NULL);
    }

    for(int i=0;i<=9;i++){                                                                      //initializing pthread_cond
        pthread_cond_init(&cv[i],NULL);
        pthread_mutex_init(&mp[i], NULL);
    }
    pthread_attr_init(&tattr);                                                                 //initializing attribute for sceduling policy
    pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);
    pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
    pthread_barrier_init(&barrier, NULL, (NUM_THREADS+1));                                     //initializing the barrier
    // To initialize the thread pthread_create is used
    // first parameter is pointer to t1 vairable , second is attribute 
    // third is function to exectue after thread creation,but to pass in a function as parameter we have to use pointer
    for(int i=0;i<NUM_THREADS;i++){                                                                 //creating threads over the for loop
        param.sched_priority = threads[i].priority;
        pthread_attr_setschedparam (&tattr, &param);
        if( threads[i].task_type == 0){                                                        // if loop  for checking  if the task is periodic or apriodic
            if (pthread_create(&threadId[i], &tattr, &P_routine, &threads[i])!=0) {
                perror("Failed to create thread");
                return 1;
            }

        }

        else {
            if (pthread_create(&threadId[i], &tattr, &A_routine, &threads[i])!=0) {
                perror("Failed to create thread");
            
                return 1;
            }

        }
    }
    param.sched_priority = 98;                                                                      //setting sceduler priority for keyboard event
    pthread_attr_setschedparam(&tattr, &param);                                                     //setting attribute for keyboard thread
    pthread_create(&keyboard_thread, &tattr, keyboard, NULL);                                       //creating keyboard thread


    sleep(TOTAL_TIME/1000);                                                                       //sleep 
    flag = 0;                                                                                      //setting the flag to 0
    pthread_cancel(keyboard_thread);                                                              //termination of thread

    for(int i=0;i<NUM_THREADS;i++)
    {
        if(threads[i].task_type == 1)
        {
            pthread_cond_broadcast(&cv[threads[i].event_key]);                                  //broadcasting the event after termination
        }
    }
    

    for(int i=0;i<NUM_THREADS;i++){
        if (pthread_join(threadId[i], NULL)!=0){                                                   //joining the threads
            perror("Failed to join thread");
            
        }
        else
            printf("Thread %d exitted\n", i);
    }

    pthread_join(keyboard_thread, NULL);                                                        //Joining the keyboard thread
    printf("Thread keyboard exitted\n");

    for(int i=0;i<NUM_MUTEXES;i++){
        pthread_mutex_destroy(&m[i]);                                                          //destroying the mutex
    }

    for(int i=0;i<10;i++){
        pthread_cond_destroy(&cv[i]);                                                          //destroying the cond thread
        pthread_mutex_destroy(&mp[i]);                                                         //destroying the conditional mutex
    }
    
    pthread_barrier_destroy(&barrier);                                                      //destroying barrier
    pthread_attr_destroy(&tattr);                                                               //destroying the attributes

    return 0;
}




