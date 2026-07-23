#include "codexion.h"


int dongle_cooldown(thread_data *coder, dongle *d)
{
    long current_time;
    long time_since_release;
    long time_to_wait;


    pthread_mutex_unlock(&d->mutex);
    current_time = get_time(coder->sim->start_time);
    time_since_release = current_time - d->last_used_time;
    time_to_wait = coder->sim->dongle_cooldown - time_since_release;
    if (time_to_wait > 0)
        if (!safe_sleep(coder, time_to_wait))
            return 0;
    pthread_mutex_lock(&d->mutex);
    return 1;
}

int wait_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);

    while (d->in_use || coder->info->thread_id != d->queue[0].thread_id)
    {
        pthread_cond_wait(&d->cond, &d->mutex);
        if (!sim_status(coder))
        {
            pthread_mutex_unlock(&d->mutex);
            return 0;
        }
    }

    // dongle cooldown
    if (d->last_used_time != 0)
        if (!dongle_cooldown(coder, d))
            return 0;

    d->in_use = 1;
    d->queue[0] = d->queue[1];
    d->queue_count--;
    pthread_mutex_unlock(&d->mutex);
    return 1;
}


void release_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);
    d->in_use = 0;
    d->last_used_time = get_time(coder->sim->start_time);
    pthread_cond_broadcast(&d->cond);
    pthread_mutex_unlock(&d->mutex);
}



void add_to_queue(thread_data *coder, dongle *d)
{
    long deadline1;
    long deadline2;

    pthread_mutex_lock(&d->mutex);
    d->queue[d->queue_count] = *coder->info;
    d->queue_count++;
    if (d->queue_count == 2 && strcmp(coder->sim->scheduler, "edf") == 0)
    {
        deadline1 = coder->sim->time_to_burnout + d->queue[0].last_compilation_time;
        deadline2 = coder->sim->time_to_burnout + d->queue[1].last_compilation_time;
        if (deadline2 < deadline1)
            swap(&d->queue[0], &d->queue[1]);
        else if (deadline2 == deadline1)
        {
            // Tie breakers
            if (d->queue[1].compile_count < d->queue[0].compile_count)
                swap(&d->queue[0], &d->queue[1]);
            else if (d->queue[1].compile_count == d->queue[0].compile_count)
                if (d->queue[1].thread_id < d->queue[0].thread_id)
                    swap(&d->queue[0], &d->queue[1]);
        }
    }
    pthread_mutex_unlock(&d->mutex);
}