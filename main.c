#include "codexion.h"



int main(int argc, char **argv)
{
    // to avoid unused variable warning
    (void)argc;
    (void)argv;

    simulation_data sim_data;

    if (parse_argument(argc, argv, &sim_data) == 0)
        return (1);
    // sim_data.nb_coders = 5;
    // sim_data.time_to_burnout = 301;
    // sim_data.time_to_compile = 100; // simulate compiling for 100 milliseconds
    // sim_data.time_to_debug = 1; // simulate debugging for 100 milliseconds
    // sim_data.time_to_refactor = 1; // simulate refactoring for 100 milliseconds
    // sim_data.compilations = 10;
    // sim_data.dongle_cooldown = 0; // simulate dongle cooldown for 1000 milliseconds
    // sim_data.scheduler = "fifo";
    // sim_data.sim_status = 1;
    // pthread_mutex_init(&sim_data.print_mutex, NULL);
    // pthread_mutex_init(&sim_data.stop_mutex, NULL);


    pthread_t *th = malloc(sizeof(pthread_t) * sim_data.nb_coders);
    long i;

    struct timeval start;
    gettimeofday(&start, NULL);
    sim_data.start_time = start;
    i = 0;

    thread_data *coders = malloc(sizeof(thread_data) * sim_data.nb_coders);

    dongle *dongles = malloc(sizeof(dongle) * sim_data.nb_coders);

    coder_info *info = malloc(sizeof(coder_info) * sim_data.nb_coders);

    while (i < sim_data.nb_coders)
    {
        pthread_mutex_init(&dongles[i].mutex, NULL);
        dongles[i].in_use = 0;
        dongles[i].queue_count = 0;
        dongles[i].last_used_time = 0;
        pthread_cond_init(&dongles[i].cond, NULL); //  

        info[i].last_compilation_time = 0;
        info[i].thread_id = i + 1;
        info[i].compile_count = 0;
        i++;
    }
    i = 0;


    while (i < sim_data.nb_coders)
    {
        // initialize thread data
        coders[i].info = &info[i];
        coders[i].dongle1 = &dongles[i];
        coders[i].dongle2 = &dongles[(i + 1) % sim_data.nb_coders];
        coders[i].sim = &sim_data;
        pthread_mutex_init(&coders[i].state_mutex, NULL);

        pthread_create(&th[i], NULL, &routine, &coders[i]);
        i++;
    }

    // monitor thread
    pthread_t monitor_th;
    pthread_create(&monitor_th, NULL, &monitor, coders);
    pthread_join(monitor_th, NULL);

    
    i = 0;
    while (i < sim_data.nb_coders)
    {
        pthread_join(th[i], NULL);
        i++;
    }

    for (i = 0; i < sim_data.nb_coders; i++) {
        pthread_mutex_destroy(&dongles[i].mutex);
        pthread_cond_destroy(&dongles[i].cond);
        pthread_mutex_destroy(&coders[i].state_mutex);
    }
    pthread_mutex_destroy(&sim_data.print_mutex);
    pthread_mutex_destroy(&sim_data.stop_mutex);

    free(th);
    free(coders);
    free(dongles);
    free(info);
    return 0;
}