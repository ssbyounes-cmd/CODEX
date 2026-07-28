/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:05:11 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:46:15 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wake_all(t_thread_data *coders, long nb_coders)
{
	long	i;

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

static int	victory_check(t_thread_data *coders, t_simulation_data *sim,
		long finished_coders, int i)
{
	pthread_mutex_lock(&coders[i].state_mutex);
	if (coders[i].info->compile_count >= sim->compilations)
		finished_coders++;
	pthread_mutex_unlock(&coders[i].state_mutex);
	if (finished_coders >= sim->nb_coders)
	{
		pthread_mutex_lock(&sim->stop_mutex);
		sim->sim_status = 0;
		pthread_mutex_unlock(&sim->stop_mutex);
		wake_all(coders, sim->nb_coders);
	}
	return (finished_coders);
}

static int	death_check(t_thread_data *coders, t_simulation_data *sim, int i)
{
	long	c_time;

	c_time = get_time(sim->start_time);
	pthread_mutex_lock(&coders[i].state_mutex);
	if (c_time - coders[i].info->last_compilation_time > sim->time_to_burnout)
	{
		pthread_mutex_unlock(&coders[i].state_mutex);
		pthread_mutex_lock(&sim->stop_mutex);
		sim->sim_status = 0;
		pthread_mutex_unlock(&sim->stop_mutex);
		safe_print(&coders[i], "%lu %d burned out\n");
		wake_all(coders, sim->nb_coders);
		return (0);
	}
	pthread_mutex_unlock(&coders[i].state_mutex);
	return (1);
}

void	*monitor(void *arg)
{
	t_thread_data		*coders;
	t_simulation_data	*sim;
	long				i;
	long				finished_coders;

	coders = (t_thread_data *)arg;
	sim = coders[0].sim;
	i = 0;
	while (1)
	{
		finished_coders = 0;
		i = 0;
		while (i < sim->nb_coders)
		{
			finished_coders = victory_check(coders, sim, finished_coders, i);
			if (finished_coders >= sim->nb_coders)
				return (NULL);
			if (!death_check(coders, sim, i))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
}
