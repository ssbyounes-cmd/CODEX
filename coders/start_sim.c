/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start_sim.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:04:59 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:42:24 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_threads(pthread_t *th, int coders_tojoin)
{
	long	i;

	i = 0;
	while (i < coders_tojoin)
	{
		pthread_join(th[i], NULL);
		i++;
	}
}

static int	create_coders(t_simulation_data *sim_data, t_data_alloc *all_data)
{
	long	i;

	i = 0;
	while (i < sim_data->nb_coders)
	{
		all_data->coders[i].info = &all_data->info[i];
		all_data->coders[i].dongle1 = &all_data->dongles[i];
		all_data->coders[i].dongle2 = &all_data->dongles[(i + 1)
			% sim_data->nb_coders];
		all_data->coders[i].sim = sim_data;
		if (pthread_create(&all_data->th[i], NULL, &routine,
				&all_data->coders[i]) != 0)
		{
			pthread_mutex_lock(&sim_data->stop_mutex);
			sim_data->sim_status = 0;
			pthread_mutex_unlock(&sim_data->stop_mutex);
			join_threads(all_data->th, i);
			return (0);
		}
		i++;
	}
	return (1);
}

int	start_simulation(t_simulation_data *sim_data, t_data_alloc *all_data)
{
	pthread_t		monitor_th;
	struct timeval	start;

	gettimeofday(&start, NULL);
	sim_data->start_time = start;
	if (!create_coders(sim_data, all_data))
		return (0);
	if (pthread_create(&monitor_th, NULL, &monitor, all_data->coders) != 0)
	{
		pthread_mutex_lock(&sim_data->stop_mutex);
		sim_data->sim_status = 0;
		pthread_mutex_unlock(&sim_data->stop_mutex);
		join_threads(all_data->th, sim_data->nb_coders);
		return (0);
	}
	pthread_join(monitor_th, NULL);
	join_threads(all_data->th, sim_data->nb_coders);
	return (1);
}
