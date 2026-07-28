/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:05:24 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:44:15 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct simulation_data
{
	long				nb_coders;
	char				*scheduler;
	long				time_to_burnout;
	long				time_to_compile;
	long				time_to_debug;
	long				time_to_refactor;
	long				compilations;
	long				dongle_cooldown;
	struct timeval		start_time;
	pthread_mutex_t		print_mutex;
	pthread_mutex_t		stop_mutex;
	int					print_ready;
	int					stop_ready;
	int					sim_status;
}						t_simulation_data;

typedef struct coder_info
{
	long				last_compilation_time;
	long				thread_id;
	long				compile_count;
}						t_coder_info;

typedef struct dongle
{
	int					in_use;
	t_coder_info		queue[2];
	int					queue_count;
	long				last_used_time;
	pthread_mutex_t		mutex;
	pthread_cond_t		cond;
	int					mutex_ready;
	int					cond_ready;
}						t_dongle;

typedef struct thread_data
{
	t_dongle			*dongle1;
	t_dongle			*dongle2;
	t_coder_info		*info;

	t_simulation_data	*sim;
	pthread_mutex_t		state_mutex;
	int					mutex_ready;
}						t_thread_data;

typedef struct data_alloc
{
	pthread_t			*th;
	t_thread_data		*coders;
	t_dongle			*dongles;
	t_coder_info		*info;
}						t_data_alloc;

/* parsing */
int		parse_argument(char **argv, t_simulation_data *sim_data);

/* initialization */
int		init_simulation(t_simulation_data *sim_data, t_data_alloc *all_data);
int		start_simulation(t_simulation_data *sim_data, t_data_alloc *all_data);

/* simulation */
void	*routine(void *coders);
int		wait_dongle(t_thread_data *coder, t_dongle *dongle);
void	release_dongle(t_thread_data *coder, t_dongle *dongle);
void	add_to_queue(t_thread_data *coder, t_dongle *dongle);

/* cleanup */
void	cleanup_simulation(t_simulation_data *sim_data, t_data_alloc *all_data);

/* monitor */
void	*monitor(void *arg);

/* helpers */
int		sim_status(t_thread_data *coder);
long	get_time(struct timeval start_time);
void	safe_print(t_thread_data *data, char *text);
int		safe_sleep(t_thread_data *coder, long ms);
void	swap(t_coder_info *a, t_coder_info *b);

#endif