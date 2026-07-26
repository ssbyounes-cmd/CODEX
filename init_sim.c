#include "codexion.h"



static int allocate(simulation_data *sim_data, data_alloc *all_data)
{
	long i;

	if (pthread_mutex_init(&sim_data->print_mutex, NULL) != 0)
		return (0);
	sim_data->print_ready = 1;
    if (pthread_mutex_init(&sim_data->stop_mutex, NULL) != 0)
		return (0);
	sim_data->stop_ready = 1;
	all_data->th = malloc(sizeof(pthread_t) * sim_data->nb_coders);
	all_data->coders = malloc(sizeof(thread_data) * sim_data->nb_coders);
	all_data->dongles = malloc(sizeof(dongle) * sim_data->nb_coders);
	all_data->info = malloc(sizeof(coder_info) * sim_data->nb_coders);
	if (!all_data->th || !all_data->coders || !all_data->dongles || !all_data->info)
		return (0);
	i = 0;
	while (i < sim_data->nb_coders)
	{
		all_data->dongles[i].mutex_ready = 0;
		all_data->dongles[i].cond_ready = 0;
		all_data->coders[i].mutex_ready = 0;
		i++;
	}
	return 1;
}



int	init_simulation(simulation_data *sim_data, data_alloc *all_data)
{
	long	i;
	
	if (!allocate(sim_data, all_data))
		return (0);
	i = 0;
	while (i < sim_data->nb_coders)
	{
		if (pthread_mutex_init(&all_data->dongles[i].mutex, NULL) != 0)
			return (0);
		all_data->dongles[i].mutex_ready = 1;
		if (pthread_cond_init(&all_data->dongles[i].cond, NULL) != 0)
			return (0);
		all_data->dongles[i].cond_ready = 1;
		if (pthread_mutex_init(&all_data->coders[i].state_mutex, NULL) != 0)
			return (0);
		all_data->coders[i].mutex_ready = 1;
		all_data->dongles[i].in_use = 0;
		all_data->dongles[i].queue_count = 0;
		all_data->dongles[i].last_used_time = 0;
		all_data->info[i].last_compilation_time = 0;
		all_data->info[i].thread_id = i + 1;
		all_data->info[i].compile_count = 0;
		i++;
	}
	return (1);
}