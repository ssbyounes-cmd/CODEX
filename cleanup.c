#include "codexion.h"


static void clean_allocation(data_alloc *all_data)
{
    if (all_data->coders)
        free(all_data->coders);
    if (all_data->dongles)
        free(all_data->dongles);
    if (all_data->info)
        free(all_data->info);
    if (all_data->th)
        free(all_data->th);
}


void cleanup_simulation(simulation_data *sim_data, data_alloc *all_data)
{
    int i;

    i = 0;
    if (all_data->dongles && all_data->coders && all_data->info && all_data->th)
    {
        while (i < sim_data->nb_coders) 
        {
            if (all_data->dongles[i].mutex_ready)
                pthread_mutex_destroy(&all_data->dongles[i].mutex);
            if (all_data->dongles[i].cond_ready)
                pthread_cond_destroy(&all_data->dongles[i].cond);
            if (all_data->coders[i].mutex_ready)
                pthread_mutex_destroy(&all_data->coders[i].state_mutex);
            i++;
        }
    }

    if (sim_data->print_ready)
        pthread_mutex_destroy(&sim_data->print_mutex);
    if (sim_data->stop_ready)
        pthread_mutex_destroy(&sim_data->stop_mutex);

    clean_allocation(all_data);
}