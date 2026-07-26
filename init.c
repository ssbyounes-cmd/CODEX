#include "codexion.h"



int	init_simulation(simulation_data *sim_data, data_alloc *all_data)
{
	long	i;

	all_data->th = malloc(sizeof(pthread_t) * sim_data->nb_coders);
	all_data->coders = malloc(sizeof(thread_data) * sim_data->nb_coders);
	all_data->dongles = malloc(sizeof(dongle) * sim_data->nb_coders);
	all_data->info = malloc(sizeof(coder_info) * sim_data->nb_coders);
	if (!all_data->th || !all_data->coders || !all_data->dongles || !all_data->info)
		return (0);
	i = 0;
	while (i < sim_data->nb_coders)
	{
		if (pthread_mutex_init(&all_data->dongles[i].mutex, NULL) != 0)
			return (0);
		if (pthread_cond_init(&all_data->dongles[i].cond, NULL) != 0)
			return (0);
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

static void	join_all_threads(simulation_data *sim_data, pthread_t *th,
		pthread_t monitor_th)
{
	long	i;

	i = 0;
	pthread_join(monitor_th, NULL);
	while (i < sim_data->nb_coders)
	{
		pthread_join(th[i], NULL);
		i++;
	}
}

int	start_simulation(simulation_data *sim_data, data_alloc *all_data)
{
	pthread_t monitor_th;
	struct timeval start;
	long i;

	i = 0;
	gettimeofday(&start, NULL);
	sim_data->start_time = start;
    pthread_mutex_init(&sim_data->print_mutex, NULL);
    pthread_mutex_init(&sim_data->stop_mutex, NULL);
	while (i < sim_data->nb_coders)
	{
		all_data->coders[i].info = &all_data->info[i];
		all_data->coders[i].dongle1 = &all_data->dongles[i];
		all_data->coders[i].dongle2 = &all_data->dongles[(i + 1) % sim_data->nb_coders];
		all_data->coders[i].sim = sim_data;
		if (pthread_mutex_init(&all_data->coders[i].state_mutex, NULL) != 0)
			return (0);
		if (pthread_create(&all_data->th[i], NULL, &routine, &all_data->coders[i]) != 0)
			return (0);
		i++;
	}
	if (pthread_create(&monitor_th, NULL, &monitor, all_data->coders) != 0)
		return (0);

	join_all_threads(sim_data, all_data->th, monitor_th);
	// pthread_join(monitor_th, NULL);
	// i = 0;
	// while (i < sim_data->nb_coders)
	// {
	//     pthread_join(th[i], NULL);
	//     i++;
	// }
	return (1);
}