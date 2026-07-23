#include "codexion.h"


static int atoi_safe(char *arg, long *result)
{
    int i;
    int sign;
    long num;

    i = 0;
    sign = 1;
    num = 0;
    while (arg[i] >= 9 && arg[i] <= 13)
        i++;
    if (arg[i] == '-')
        sign = -1;
    if (arg[i] == '+' || arg[i] == '-')
        i++;
    while (arg[i])
    {
        if (!(arg[i] >= '0' && arg[i] <= '9'))
            return -1;
        num = num * 10 + (arg[i] - '0');
        if (num > 2147483647)
            return 0;
        i++;
    }
    *result = num * sign;
    return 1;
}

void fill_data(simulation_data *sim_data, long *parsed)
{
    sim_data->nb_coders = parsed[0];
    sim_data->time_to_burnout = parsed[1];
    sim_data->time_to_compile = parsed[2];
    sim_data->time_to_debug = parsed[3];
    sim_data->time_to_refactor = parsed[4];
    sim_data->compilations = parsed[5];
    sim_data->dongle_cooldown = parsed[6];
    sim_data->sim_status = 1;
    pthread_mutex_init(&sim_data->print_mutex, NULL);
    pthread_mutex_init(&sim_data->stop_mutex, NULL);
}

int check_times(simulation_data *sim_data)
{
    if (sim_data->time_to_burnout <= 0)
    {
        fprintf(stderr, "Error: Time to burnout must be > 0\n");
        return 0;
    }
    if (sim_data->time_to_compile <= 0)
    {
        fprintf(stderr, "Error: Time to compile must be > 0\n");
        return 0;
    }
    if (sim_data->time_to_debug <= 0)
    {
        fprintf(stderr, "Error: Time to debug must be > 0\n");
        return 0;
    }
    if (sim_data->time_to_refactor <= 0)
    {
        fprintf(stderr, "Error: Time to refactor must be > 0\n");
        return 0;
    }
    return 1;
}


int check_argument(simulation_data *sim_data)
{
    if (sim_data->nb_coders <= 0)
    {
        fprintf(stderr, "Error: Number of coders must be > 0\n");
        return 0;
    }
    if (!check_times(sim_data))
        return 0;
    if (sim_data->compilations <= 0)
    {
        fprintf(stderr, "Error: Number of compilations must be > 0\n");
        return 0;
    }
    if (sim_data->dongle_cooldown < 0)
    {
        fprintf(stderr, "Error: Dongle cooldown must be >= 0\n");
        return 0;
    }
    if (strcmp(sim_data->scheduler, "fifo") != 0 
    && strcmp(sim_data->scheduler, "edf") != 0)
    {
        fprintf(stderr, "Error: Scheduler must be either 'fifo' or 'edf'\n");
        return 0;
    }
    return 1;
}

int parse_argument(int argc, char **argv, simulation_data *sim_data)
{
    long parsed[7];
    int i;

    if (argc != 9)
    {
        fprintf(stderr, "Usage: %s <nb_coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <compilations> <dongle_cooldown> <scheduler>\n", argv[0]);
        return 0;
    }
    i = 0;
    while (i < 7)
    {
        if (atoi_safe(argv[i + 1], &parsed[i]) != 1)
        {
            if (atoi_safe(argv[i + 1], &parsed[i]) == 0)
                fprintf(stderr, "Error: Argument %d out of range\n", i + 1);
            else if (atoi_safe(argv[i + 1], &parsed[i]) == -1)
                fprintf(stderr, "Error: Argument %d invalid\n", i + 1);
            return 0;
        }
        i++;
    }
    fill_data(sim_data, parsed);
    sim_data->scheduler = argv[8];
    return check_argument(sim_data);
}