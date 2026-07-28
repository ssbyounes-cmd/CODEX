/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_sim.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:05:20 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:37:48 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	prep(t_simulation_data *sim_data, t_data_alloc *all_data)
{
	all_data->th = NULL;
	all_data->coders = NULL;
	all_data->dongles = NULL;
	all_data->info = NULL;
	sim_data->print_ready = 0;
	sim_data->stop_ready = 0;
	if (pthread_mutex_init(&sim_data->print_mutex, NULL) != 0)
		return (0);
	sim_data->print_ready = 1;
	if (pthread_mutex_init(&sim_data->stop_mutex, NULL) != 0)
		return (0);
	sim_data->stop_ready = 1;
	return (1);
}

static int	allocate(t_simulation_data *sim_data, t_data_alloc *all_data)
{
	long	i;

	if (!prep(sim_data, all_data))
		return (0);
	all_data->th = malloc(sizeof(pthread_t) * sim_data->nb_coders);
	all_data->coders = malloc(sizeof(t_thread_data) * sim_data->nb_coders);
	all_data->dongles = malloc(sizeof(t_dongle) * sim_data->nb_coders);
	all_data->info = malloc(sizeof(t_coder_info) * sim_data->nb_coders);
	if (!all_data->th || !all_data->coders || !all_data->dongles
		|| !all_data->info)
		return (0);
	i = 0;
	while (i < sim_data->nb_coders)
	{
		all_data->dongles[i].mutex_ready = 0;
		all_data->dongles[i].cond_ready = 0;
		all_data->coders[i].mutex_ready = 0;
		i++;
	}
	return (1);
}

int	init_simulation(t_simulation_data *sim_data, t_data_alloc *all_data)
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
