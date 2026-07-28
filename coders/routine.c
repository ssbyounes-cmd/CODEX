/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:05:04 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:42:06 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap_dongles(t_thread_data *coder)
{
	t_dongle	*temp;

	temp = coder->dongle1;
	coder->dongle1 = coder->dongle2;
	coder->dongle2 = temp;
	usleep(100);
}

static int	take_dongles(t_thread_data *coder)
{
	add_to_queue(coder, coder->dongle1);
	add_to_queue(coder, coder->dongle2);
	if (!wait_dongle(coder, coder->dongle1))
		return (0);
	safe_print(coder, "%lu %d has taken a dongle\n");
	if (!wait_dongle(coder, coder->dongle2))
		return (0);
	safe_print(coder, "%lu %d has taken a dongle\n");
	return (1);
}

static int	perform_compilation(t_thread_data *coder)
{
	pthread_mutex_lock(&coder->state_mutex);
	coder->info->last_compilation_time = get_time(coder->sim->start_time);
	pthread_mutex_unlock(&coder->state_mutex);
	safe_print(coder, "%lu %d is compiling\n");
	if (!safe_sleep(coder, coder->sim->time_to_compile))
		return (0);
	pthread_mutex_lock(&coder->state_mutex);
	coder->info->compile_count++;
	pthread_mutex_unlock(&coder->state_mutex);
	release_dongle(coder, coder->dongle1);
	release_dongle(coder, coder->dongle2);
	return (1);
}

static int	debug_and_refactor(t_thread_data *coder)
{
	safe_print(coder, "%lu %d is debugging\n");
	if (!safe_sleep(coder, coder->sim->time_to_debug))
		return (0);
	safe_print(coder, "%lu %d is refactoring\n");
	if (!safe_sleep(coder, coder->sim->time_to_refactor))
		return (0);
	return (1);
}

void	*routine(void *coders)
{
	t_thread_data	*coder;
	int				compilations;

	coder = (t_thread_data *)coders;
	compilations = coder->sim->compilations;
	if (coder->info->thread_id % 2 == 0)
		swap_dongles(coder);
	while (compilations)
	{
		if (!take_dongles(coder))
			return (NULL);
		if (!perform_compilation(coder))
			return (NULL);
		if (!debug_and_refactor(coder))
			return (NULL);
		compilations--;
	}
	return (NULL);
}
