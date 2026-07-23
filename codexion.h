#ifndef codexion_h
#define codexion_h

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>



typedef struct simulation_data {
    long nb_coders;
    char *scheduler;
    long time_to_burnout;
    long time_to_compile;
    long time_to_debug;
    long time_to_refactor;
    long compilations;
    long dongle_cooldown;
    struct timeval start_time;
    pthread_mutex_t print_mutex;
    pthread_mutex_t stop_mutex;
    int sim_status;
} simulation_data;


typedef struct coder_info {
    long last_compilation_time;
    long thread_id;
    long compile_count;
} coder_info;


typedef struct dongle {
    int in_use;
    coder_info queue[2];
    int queue_count;
    long last_used_time;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} dongle;



typedef struct thread_data {
    dongle *dongle1;
    dongle *dongle2;
    coder_info *info;

    simulation_data *sim;
    pthread_mutex_t state_mutex;
} thread_data;


// helpers
int sim_status(thread_data *coder);
long get_time(struct timeval start_time);
void safe_print(thread_data *data, char *text);
int safe_sleep(thread_data *coder, long ms);
void swap(coder_info *a, coder_info *b);

void *routine(void *coders);
void *monitor(void *arg);
void wake_all(thread_data *coders, long nb_coders);

int wait_dongle(thread_data *coder, dongle *dongle);
void release_dongle(thread_data *coder, dongle *dongle);
void add_to_queue(thread_data *coder, dongle *dongle);

int parse_argument(int argc, char **argv, simulation_data *sim_data);


#endif