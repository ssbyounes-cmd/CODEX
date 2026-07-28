/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ychoucho <ychoucho@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 20:05:15 by ychoucho          #+#    #+#             */
/*   Updated: 2026/07/27 21:43:56 by ychoucho         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_simulation_data	sim_data;
	t_data_alloc		all_data;

	if (argc != 9)
	{
		fprintf(stderr, "Usage: %s <nb_coders><time_to_burnout>"
			"<time_to_compile><time_to_debug><time_to_refactor>"
			"<compilations><dongle_cooldown><scheduler>\n",
			argv[0]);
		return (0);
	}
	if (parse_argument(argv, &sim_data) == 0)
		return (1);
	if (init_simulation(&sim_data, &all_data) == 0)
	{
		cleanup_simulation(&sim_data, &all_data);
		return (1);
	}
	if (start_simulation(&sim_data, &all_data) == 0)
	{
		cleanup_simulation(&sim_data, &all_data);
		return (1);
	}
	cleanup_simulation(&sim_data, &all_data);
	return (0);
}
