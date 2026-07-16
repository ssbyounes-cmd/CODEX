#include "codexion.h"


void wake_all(thread_data *coders, unsigned long nb_coders)
{
    int i;

    for (i = 0; i < nb_coders; i++)
    {
        pthread_mutex_lock(&coders[i].dongle1->mutex);
        pthread_cond_broadcast(&coders[i].dongle1->cond);
        pthread_mutex_unlock(&coders[i].dongle1->mutex);

        pthread_mutex_lock(&coders[i].dongle2->mutex);
        pthread_cond_broadcast(&coders[i].dongle2->cond);
        pthread_mutex_unlock(&coders[i].dongle2->mutex);
    }
}




void *monitor(void *arg)
{
    thread_data *coders = (thread_data *) arg;
    simulation_data *sim = coders[0].sim;
    int i;
    unsigned long c_time;

    while (1)
    {
        i = 0; 
        while (i < sim->nb_coders)
        {
            c_time = get_time(sim->start_time);
            if (c_time - coders[i].info->last_compilation_time > sim->time_to_burnout)
            {
                pthread_mutex_lock(&sim->stop_mutex);
                sim->stop = 1;
                pthread_mutex_unlock(&sim->stop_mutex);
                safe_print(&coders[i], "%lu %d burned out\n");
                wake_all(coders, sim->nb_coders);
                return NULL;
            }
        }
        usleep(1000);
    }
}