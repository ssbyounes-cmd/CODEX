#ifndef codexion_h
#define codexion_h

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>



typedef struct simulation_data {
    unsigned long nb_coders;
    char *scheduler;
    unsigned long time_to_burnout;
    unsigned long time_to_compile;
    unsigned long compilations;
    struct timeval start_time;
    pthread_mutex_t print_mutex;
} simulation_data;


typedef struct coder_info {
    int last_compilation_time;
    int thread_id;
} coder_info;


typedef struct dongle {
    int in_use;
    coder_info queue[2];
    int queue_count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} dongle;



typedef struct thread_data {
    dongle *dongle1;
    dongle *dongle2;
    coder_info *info;

    simulation_data *sim;
} thread_data;


#endif