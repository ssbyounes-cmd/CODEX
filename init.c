#include "codexion.h"


int init_simulation(simulation_data *sim_data, thread_data *coders, dongle *dongles, coder_info *info, pthread_t *th)
{
    long i;

    th = malloc(sizeof(pthread_t) * sim_data->nb_coders);
    coders = malloc(sizeof(thread_data) * sim_data->nb_coders);
    dongles = malloc(sizeof(dongle) * sim_data->nb_coders);
    info = malloc(sizeof(coder_info) * sim_data->nb_coders);
    if (!th || !coders || !dongles || !info)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return (0);
    }
    i = 0;
    while (i < sim_data->nb_coders)
    {
        pthread_mutex_init(&dongles[i].mutex, NULL);
        pthread_cond_init(&dongles[i].cond, NULL);
        dongles[i].in_use = 0;
        dongles[i].queue_count = 0;
        dongles[i].last_used_time = 0;
        info[i].last_compilation_time = 0;
        info[i].thread_id = i + 1;
        info[i].compile_count = 0;
        i++;
    }
    return (1);
}