#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>


typedef struct dongle {
    int in_use;
    int queue[2];
    int queue_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} dongle;


typedef struct thread_data {
    int thread_id;
    dongle *dongle1;
    dongle *dongle2;
    struct timeval start_time;
    pthread_mutex_t *print_mutex;
    int last_compilation_time;
} thread_data;




unsigned long get_time(struct timeval start_time)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    unsigned long milliseconds = (current_time.tv_sec - start_time.tv_sec) * 1000 + (current_time.tv_usec - start_time.tv_usec) / 1000;
    return milliseconds;
}


void safe_print(thread_data *data, char *text)
{
    pthread_mutex_lock(data->print_mutex);
    printf(text, get_time(data->start_time), data->thread_id);
    pthread_mutex_unlock(data->print_mutex);
}


void take_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);
    d->queue[d->queue_count] = coder->thread_id; // add the thread ID to the queue
    d->queue_count++;
    while (d->in_use)
        pthread_cond_wait(&d->cond, &d->mutex); // release the mutex and wait for the condition variable to be signaled

    d->in_use = 1;
    d->queue]
    pthread_mutex_unlock(&d->mutex);
}
// coders has two dongles each..  We have to protect taking and releasing otherwise, we'd face data race.
void release_dongle(dongle *d)
{
    pthread_mutex_lock(&d->mutex);
    d->in_use = 0;
    pthread_cond_broadcast(&d->cond); // signal the waiting thread that the dongle is now available
    pthread_mutex_unlock(&d->mutex);
}

void *routine(void *coders)
{
    thread_data *coder = (thread_data *)coders;
    int current_time = 0;
    int simulation = 2;
    int swap_flag = 0;

    while(simulation)
    {

        if ((coder->thread_id % 2 == 0) && (swap_flag == 0))
        {
            dongle *temp = coder->dongle1;
            coder->dongle1 = coder->dongle2;
            coder->dongle2 = temp;
            swap_flag = 1;
            usleep(100);
        }

        take_dongle(coder, coder->dongle1);
        safe_print(coder, "%lu %d has taken a first dongle\n");
        take_dongle(coder, coder->dongle2);
        safe_print(coder, "%lu %d has taken a second dongle\n");


        coder->last_compilation_time = get_time(coder->start_time);
        safe_print(coder, "%lu %d is compiling\n");
        usleep(100000); // simulate compiling for 100 milliseconds
        // release dongles after compiling
        release_dongle(coder->dongle1);
        release_dongle(coder->dongle2);
        simulation--;
        // break;
    }
        
}

int main(int argc, char **argv)
{
    int nb_coders = 4;
    char *scheduler = "fifo";
    int time_to_burnout = 500;
    int compilations = 2;


    pthread_t th[nb_coders];
    pthread_mutex_t print_mutex;
    int i;

    pthread_mutex_init(&print_mutex, NULL);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    i = 0;


    thread_data coders[nb_coders];

    dongle dongle[nb_coders];

    for (i = 0; i < nb_coders; i++){
        pthread_mutex_init(&dongle[i].mutex, NULL);
        dongle[i].in_use = 0;
        dongle[i].queue_count = 0;
        pthread_cond_init(&dongle[i].cond, NULL); //  
    }
    i = 0;

    while (i < nb_coders)
    {

        // initialize thread data
        coders[i].thread_id = i + 1;
        coders[i].dongle1 = &dongle[i];
        coders[i].dongle2 = &dongle[(i + 1) % nb_coders];
        coders[i].start_time = start;
        coders[i].print_mutex = &print_mutex;

        pthread_create(&th[i], NULL, &routine, &coders[i]);
        i++;
    }
    i = 0;
    while (i < nb_coders)
    {
        pthread_join(th[i], NULL);
        i++;
    }

    for (i = 0; i < nb_coders; i++) {
        pthread_mutex_destroy(&dongle[i].mutex);
        pthread_cond_destroy(&dongle[i].cond);
    }
    return 0;
}