#include "codexion.h"


void cleanup_simulation(simulation_data *sim_data, data_alloc *all_data)
{
    int i;

    i = 0;
    while (i < sim_data->nb_coders) 
    {
        pthread_mutex_destroy(&all_data->dongles[i].mutex);
        pthread_cond_destroy(&all_data->dongles[i].cond);
        pthread_mutex_destroy(&all_data->coders[i].state_mutex);
        i++;
    }
    pthread_mutex_destroy(&sim_data->print_mutex);
    pthread_mutex_destroy(&sim_data->stop_mutex);


    if (all_data->coders)
        free(all_data->coders);
    if (all_data->dongles)
        free(all_data->dongles);
    if (all_data->info)
        free(all_data->info);
    if (all_data->th)
        free(all_data->th);
}