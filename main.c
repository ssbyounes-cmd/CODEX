#include "codexion.h"



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
    printf(text, get_time(data->sim->start_time), data->info->thread_id);
    pthread_mutex_unlock(&data->sim->print_mutex);
}

void add_to_queue(thread_data *coder, dongle *d)
{
    unsigned long deadline1;
    unsigned long deadline2;
    coder_info temp;

    pthread_mutex_lock(&d->mutex);

    d->queue[d->queue_count] = *coder->info; // add the thread ID to the queue
    d->queue_count++;
    if (d->queue_count == 2 && strcmp(coder->sim->scheduler, "edf") == 0)
    {
        deadline1 = coder->sim->time_to_burnout + d->queue[0].last_compilation_time;
        deadline2 = coder->sim->time_to_burnout + d->queue[1].last_compilation_time;
        if (deadline2 < deadline1)
        {
            // printf("ba33bouoo\n");
            temp = d->queue[0];
            d->queue[0] = d->queue[1];
            d->queue[1] = temp;
        }
        else if (deadline2 == deadline1)
            if (d->queue[1].thread_id < d->queue[0].thread_id)
            {
                temp = d->queue[0];
                d->queue[0] = d->queue[1];
                d->queue[1] = temp;
            }
    }
    pthread_mutex_unlock(&d->mutex);
}

void wait_dongle(thread_data *coder, dongle *d)
{
    pthread_mutex_lock(&d->mutex);

    while (d->in_use || coder->info->thread_id != d->queue[0].thread_id) // check if the dongle is in use or if the thread is not at the front of the queue  
        pthread_cond_wait(&d->cond, &d->mutex); // release the mutex and wait for the condition variable to be signaled



    // 2. It is our turn! But we must drop the clipboard to check the heat.
    if (d->last_used_time != 0)
    {
        pthread_mutex_unlock(&d->mutex);
        unsigned long current_time = get_time(coder->sim->start_time);
        long time_since_release = current_time - d->last_used_time;
        long time_to_wait = coder->sim->dongle_cooldown - time_since_release;
        if (time_to_wait > 0)
        {
            usleep(time_to_wait * 1000);
        }
        // 5. Cooldown is over. Grab the clipboard one last time to claim it.
        pthread_mutex_lock(&d->mutex);
    }



    d->in_use = 1;
    d->queue[0] = d->queue[1]; // shift the queue to the left
    d->queue_count--; // remove the thread ID from the queue
    pthread_mutex_unlock(&d->mutex);
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
    int current_time = 0;
    int swap_flag = 0;
    int compilations = coder->sim->compilations;

    while(compilations)
    {
        if ((coder->info->thread_id % 2 == 0) && (swap_flag == 0))
        {
            dongle *temp = coder->dongle1;
            coder->dongle1 = coder->dongle2;
            coder->dongle2 = temp;
            swap_flag = 1;
            usleep(100);
        }

        add_to_queue(coder, coder->dongle1);
        // simulate some work before trying to acquire the second dongle
        add_to_queue(coder, coder->dongle2);


        wait_dongle(coder, coder->dongle1);
        safe_print(coder, "%lu %d has taken a first dongle\n");
        wait_dongle(coder, coder->dongle2);
        safe_print(coder, "%lu %d has taken a second dongle\n");


        coder->info->last_compilation_time = get_time(coder->sim->start_time);
        safe_print(coder, "%lu %d is compiling\n");
        usleep(coder->sim->time_to_compile * 1000); // simulate compiling for 100 milliseconds
        // release dongles after compiling
        release_dongle(coder, coder->dongle1);
        release_dongle(coder, coder->dongle2);
        usleep(1000); // simulate debbuging
        usleep(1000); // simulate refractoring
        compilations--;
        // break;
    }
        
}

int main(int argc, char **argv)
{

    simulation_data sim_data;

    sim_data.nb_coders = 3;
    sim_data.scheduler = "fifo";
    sim_data.time_to_burnout = 500;
    sim_data.time_to_compile = 100; // simulate compiling for 100 milliseconds
    sim_data.compilations = 1;
    sim_data.dongle_cooldown = 600; // simulate dongle cooldown for 1000 milliseconds
    pthread_mutex_init(&sim_data.print_mutex, NULL);


    pthread_t th[sim_data.nb_coders];
    int i;

    struct timeval start;
    gettimeofday(&start, NULL);
    sim_data.start_time = start;
    i = 0;

    thread_data coders[sim_data.nb_coders];

    dongle dongle[sim_data.nb_coders];

    coder_info info[sim_data.nb_coders];

    for (i = 0; i < sim_data.nb_coders; i++){
        pthread_mutex_init(&dongle[i].mutex, NULL);
        dongle[i].in_use = 0;
        dongle[i].queue_count = 0;
        dongle[i].last_used_time = 0;
        pthread_cond_init(&dongle[i].cond, NULL); //  

        info[i].last_compilation_time = 0;
        info[i].thread_id = i + 1;
    }
    i = 0;


    while (i < sim_data.nb_coders)
    {
        // initialize thread data
        coders[i].info = &info[i];
        coders[i].dongle1 = &dongle[i];
        coders[i].dongle2 = &dongle[(i + 1) % sim_data.nb_coders];
        coders[i].sim = &sim_data;

        pthread_create(&th[i], NULL, &routine, &coders[i]);
        i++;
    }
    i = 0;
    while (i < sim_data.nb_coders)
    {
        pthread_join(th[i], NULL);
        i++;
    }

    for (i = 0; i < sim_data.nb_coders; i++) {
        pthread_mutex_destroy(&dongle[i].mutex);
        pthread_cond_destroy(&dongle[i].cond);
    }
    return 0;
}