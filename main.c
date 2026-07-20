#include "codexion.h"


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
    {
        pthread_mutex_unlock(&d->mutex);
        unsigned long current_time = get_time(coder->sim->start_time);
        long time_since_release = current_time - d->last_used_time;
        long time_to_wait = coder->sim->dongle_cooldown - time_since_release;
        if (time_to_wait > 0)
            if (!safe_sleep(coder, time_to_wait))
                return 0;
        // 5. Cooldown is over. Grab the clipboard one last time to claim it.
        pthread_mutex_lock(&d->mutex);
    }



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

void *routine(void *coders)
{
    thread_data *coder = (thread_data *)coders;
    int compilations = coder->sim->compilations;

    if (coder->info->thread_id % 2 == 0)
    {
        dongle *temp = coder->dongle1;
        coder->dongle1 = coder->dongle2;
        coder->dongle2 = temp;
        usleep(100);
    }

    while (compilations)
    {

        // queue registering
        add_to_queue(coder, coder->dongle1);
        add_to_queue(coder, coder->dongle2);

        // taking the dongles
        if (!wait_dongle(coder, coder->dongle1))
            return NULL;
        safe_print(coder, "%lu %d has taken a dongle\n");

        if (!wait_dongle(coder, coder->dongle2))
            return NULL;
        safe_print(coder, "%lu %d has taken a dongle\n");

        // compiling
        pthread_mutex_lock(&coder->state_mutex);
        coder->info->last_compilation_time = get_time(coder->sim->start_time);
        pthread_mutex_unlock(&coder->state_mutex);
        safe_print(coder, "%lu %d is compiling\n");
        if (!safe_sleep(coder, coder->sim->time_to_compile))
            return NULL;

        pthread_mutex_lock(&coder->state_mutex);
        coder->info->compile_count++;
        pthread_mutex_unlock(&coder->state_mutex);

        // release dongles after compiling
        release_dongle(coder, coder->dongle1);
        release_dongle(coder, coder->dongle2);

        // debugging and refractoring
        safe_print(coder, "%lu %d is debugging\n");
        if (!safe_sleep(coder, coder->sim->time_to_debug))
            return NULL;
        safe_print(coder, "%lu %d is refactoring\n");
        if (!safe_sleep(coder, coder->sim->time_to_refactor))
            return NULL;

        compilations--;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    // to avoid unused variable warning
    (void)argc;
    (void)argv;

    simulation_data sim_data;

    sim_data.nb_coders = 4;
    sim_data.scheduler = "edf";
    sim_data.time_to_burnout = 202;
    sim_data.time_to_compile = 100; // simulate compiling for 100 milliseconds
    sim_data.time_to_debug = 1; // simulate debugging for 100 milliseconds
    sim_data.time_to_refactor = 1; // simulate refactoring for 100 milliseconds
    sim_data.compilations = 10;
    sim_data.dongle_cooldown = 0; // simulate dongle cooldown for 1000 milliseconds
    sim_data.sim_status = 1;
    pthread_mutex_init(&sim_data.print_mutex, NULL);
    pthread_mutex_init(&sim_data.stop_mutex, NULL);


    pthread_t th[sim_data.nb_coders];
    unsigned long i;

    struct timeval start;
    gettimeofday(&start, NULL);
    sim_data.start_time = start;
    i = 0;

    thread_data coders[sim_data.nb_coders];

    dongle dongle[sim_data.nb_coders];

    coder_info info[sim_data.nb_coders];

    while (i < sim_data.nb_coders)
    {
        pthread_mutex_init(&dongle[i].mutex, NULL);
        dongle[i].in_use = 0;
        dongle[i].queue_count = 0;
        dongle[i].last_used_time = 0;
        pthread_cond_init(&dongle[i].cond, NULL); //  

        info[i].last_compilation_time = 0;
        info[i].thread_id = i + 1;
        info[i].compile_count = 0;
        i++;
    }
    i = 0;


    while (i < sim_data.nb_coders)
    {
        // initialize thread data
        coders[i].info = &info[i];
        coders[i].dongle1 = &dongle[i];
        coders[i].dongle2 = &dongle[(i + 1) % sim_data.nb_coders];
        coders[i].sim = &sim_data;
        pthread_mutex_init(&coders[i].state_mutex, NULL);

        pthread_create(&th[i], NULL, &routine, &coders[i]);
        i++;
    }

    // monitor thread
    pthread_t monitor_th;
    pthread_create(&monitor_th, NULL, &monitor, coders);
    pthread_join(monitor_th, NULL);

    
    i = 0;
    while (i < sim_data.nb_coders)
    {
        pthread_join(th[i], NULL);
        i++;
    }

    for (i = 0; i < sim_data.nb_coders; i++) {
        pthread_mutex_destroy(&dongle[i].mutex);
        pthread_cond_destroy(&dongle[i].cond);
        pthread_mutex_destroy(&coders[i].state_mutex);
    }
    pthread_mutex_destroy(&sim_data.print_mutex);
    pthread_mutex_destroy(&sim_data.stop_mutex);
    return 0;
}