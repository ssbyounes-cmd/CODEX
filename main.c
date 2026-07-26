#include "codexion.h"



int main(int argc, char **argv)
{
    simulation_data sim_data;
    data_alloc all_data;

    all_data.th = NULL;
    all_data.coders = NULL;
    all_data.dongles = NULL;
    all_data.info = NULL;

    sim_data.print_ready = 0;
    sim_data.stop_ready = 0;

    if (parse_argument(argc, argv, &sim_data) == 0)
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
    return 0;
}