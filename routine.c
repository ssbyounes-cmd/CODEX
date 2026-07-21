#include "codexion.h"

void swap_dongles(thread_data *coder)
{
    dongle *temp;

    temp = coder->dongle1;
    coder->dongle1 = coder->dongle2;
    coder->dongle2 = temp;
    usleep(100);
}


void *routine(void *coders)
{
    thread_data *coder;
    int compilations;

    coder = (thread_data *)coders;
    compilations = coder->sim->compilations;

    if (coder->info->thread_id % 2 == 0)
        swap_dongles(coder);

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