#include "codexion.h"


int dongle_cooldown(thread_data *coder, dongle *d)
{
    unsigned long current_time;
    unsigned long time_since_release;
    unsigned long time_to_wait;


    pthread_mutex_unlock(&d->mutex);
    current_time = get_time(coder->sim->start_time);
    time_since_release = current_time - d->last_used_time;
    time_to_wait = coder->sim->dongle_cooldown - time_since_release;
    if (time_to_wait > 0)
        if (!safe_sleep(coder, time_to_wait))
            return 0;
    // 5. Cooldown is over. Grab the clipboard one last time to claim it.
    pthread_mutex_lock(&d->mutex);
    return 1;
}

int wait_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);

    while (d->in_use || coder->info->thread_id != d->queue[0].thread_id) // check if the dongle is in use or if the thread is not at the front of the queue
    {
        pthread_cond_wait(&d->cond, &d->mutex); // release the mutex and wait for the condition variable to be signaled
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
    d->queue[0] = d->queue[1]; // shift the queue to the left
    d->queue_count--; // remove the thread ID from the queue
    pthread_mutex_unlock(&d->mutex);
    return 1;
}

// coders has two dongles each..  We have to protect taking and releasing otherwise, we'd face data race.
void release_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);
    d->in_use = 0;
    d->last_used_time = get_time(coder->sim->start_time);
    pthread_cond_broadcast(&d->cond); // signal the waiting thread that the dongle is now available
    pthread_mutex_unlock(&d->mutex);
}



void add_to_queue(thread_data *coder, dongle *d)
{
    unsigned long deadline1;
    unsigned long deadline2;

    pthread_mutex_lock(&d->mutex);
    d->queue[d->queue_count] = *coder->info; // add the thread ID to the queue
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