#include "codexion.h"


void swap(coder_info *a, coder_info *b)
{
    coder_info temp;

    temp = *a;
    *a = *b;
    *b = temp;
}


int check_isover(thread_data *coder)
{
    int status;

    pthread_mutex_lock(&coder->sim->stop_mutex);
    status = coder->sim->stop;
    pthread_mutex_unlock(&coder->sim->stop_mutex);
    return status;
}


unsigned long get_time(struct timeval start_time)
{
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    unsigned long milliseconds = (current_time.tv_sec - start_time.tv_sec) * 1000 + (current_time.tv_usec - start_time.tv_usec) / 1000;
    return milliseconds;
}


void safe_print(thread_data *data, char *text)
{
    pthread_mutex_lock(&data->sim->print_mutex);
    if (!check_isover(data) || strcmp(text, "%lu %d burned out\n") == 0)
        printf(text, get_time(data->sim->start_time), data->info->thread_id);
    pthread_mutex_unlock(&data->sim->print_mutex);
}


void safe_sleep(thread_data *coder, unsigned long ms)
{
    unsigned long start;

    start = get_time(coder->sim->start_time);
    while (!check_isover(coder))
    {
        // printf("%lu _ %lu = %lu\n", get_time(coder->sim->start_time), start, ms);
        if (get_time(coder->sim->start_time) - start >= ms)
            break ;
        usleep(500);
    }
}