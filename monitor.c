#include "codexion.h"


void wake_all(thread_data *coders, long nb_coders)
{
    long i;

    i = 0;
    while (i < nb_coders)
    {
        pthread_mutex_lock(&coders[i].dongle1->mutex);
        pthread_cond_broadcast(&coders[i].dongle1->cond);
        pthread_mutex_unlock(&coders[i].dongle1->mutex);

        pthread_mutex_lock(&coders[i].dongle2->mutex);
        pthread_cond_broadcast(&coders[i].dongle2->cond);
        pthread_mutex_unlock(&coders[i].dongle2->mutex);
        i++;
    }
}


int victory_check(thread_data *coders, simulation_data *sim, long finished_coders, int i)
{
    pthread_mutex_lock(&coders[i].state_mutex);
    if (coders[i].info->compile_count >= sim->compilations)
        finished_coders++;
    pthread_mutex_unlock(&coders[i].state_mutex);

    if (finished_coders >= sim->nb_coders)
    {
        pthread_mutex_lock(&sim->stop_mutex);
        sim->sim_status = 0; // 0 means STOP!
        pthread_mutex_unlock(&sim->stop_mutex);
        wake_all(coders, sim->nb_coders);
    }
    return finished_coders;
}


int death_check(thread_data *coders, simulation_data *sim, int i)
{
    long c_time;
    
    c_time = get_time(sim->start_time);
    pthread_mutex_lock(&coders[i].state_mutex);
    if (c_time - coders[i].info->last_compilation_time > sim->time_to_burnout)
    {
        pthread_mutex_unlock(&coders[i].state_mutex);
        
        pthread_mutex_lock(&sim->stop_mutex);
        sim->sim_status = 0; // 0 means STOP!
        pthread_mutex_unlock(&sim->stop_mutex);
        
        safe_print(&coders[i], "%lu %d burned out\n");
        wake_all(coders, sim->nb_coders);
        return 0;
    }
    pthread_mutex_unlock(&coders[i].state_mutex);
    return 1;
}

void *monitor(void *arg)
{
    thread_data *coders;
    simulation_data *sim;
    long i;
    long finished_coders;

    coders = (thread_data *) arg;
    sim = coders[0].sim;
    i = 0;
    while (1)
    {
        finished_coders = 0;
        i = 0; 
        while (i < sim->nb_coders)
        {
            // 1. VICTORY CHECK
            finished_coders = victory_check(coders, sim, finished_coders, i);
            if (finished_coders >= sim->nb_coders)
                return NULL;

            // 2. DEATH CHECK
            if (!death_check(coders, sim, i))
                return NULL;
            i++;
        }
        usleep(1000); // 1ms sleep to prevent CPU melting
    }
}