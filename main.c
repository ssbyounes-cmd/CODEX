#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct thread_data {
    int thread_id;
    pthread_mutex_t *dongle1;
    pthread_mutex_t *dongle2;
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




void *routine(void *arg)
{
    thread_data *data = (thread_data *)arg;
    int current_time = 0;
    int simulation = 1;
    int swap_flag = 0;

    while(simulation)
    {

        if ((data->thread_id % 2 == 0) && (swap_flag == 0))
        {
            pthread_mutex_t *temp = data->dongle1;
            data->dongle1 = data->dongle2;
            data->dongle2 = temp;
            swap_flag = 1;
            usleep(100);
        }

        if (pthread_mutex_lock(data->dongle1) == 0)
            safe_print(data, "%lu %d has taken a first dongle\n");

        if (pthread_mutex_lock(data->dongle2) == 0)
            safe_print(data, "%lu %d has taken a second dongle\n");
        

        data->last_compilation_time = get_time(data->start_time);
        safe_print(data, "%lu %d is compiling\n");
        usleep(100000); // simulate compiling for 100 milliseconds
        pthread_mutex_unlock(data->dongle1);
        pthread_mutex_unlock(data->dongle2);
        
        usleep(1000);
        simulation--;
        // break;
    }
        
}

int main(int argc, char **argv)
{
    int coders = 10;
    char *scheduler = "fifo";
    int time_to_burnout = 500;
    int compilations = 2;


    pthread_t th[coders];
    pthread_mutex_t mutex[coders];
    pthread_mutex_t print_mutex;
    int i;

    for (i = 0; i < coders; i++) {
        pthread_mutex_init(&mutex[i], NULL);
    }
    pthread_mutex_init(&print_mutex, NULL);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    i = 0;


    thread_data data[coders];


    while (i < coders)
    {
        // initialize thread data
        data[i].thread_id = i + 1;
        data[i].dongle1 = &mutex[i];
        data[i].dongle2 = &mutex[(i + 1) % coders];
        data[i].start_time = start;
        data[i].print_mutex = &print_mutex;

        pthread_create(&th[i], NULL, &routine, &data[i]);
        i++;
    }
    i = 0;
    while (i < coders)
    {
        pthread_join(th[i], NULL);
        i++;
    }

    for (i = 0; i < coders; i++) {
        pthread_mutex_destroy(&mutex[i]);
    }
    return 0;
}