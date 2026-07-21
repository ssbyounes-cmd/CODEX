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
    unsigned long nb_coders;
    char *scheduler;
    unsigned long time_to_burnout;
    unsigned long time_to_compile;
    unsigned long time_to_debug;
    unsigned long time_to_refactor;
    unsigned long compilations;
    unsigned long dongle_cooldown;
    struct timeval start_time;
    pthread_mutex_t print_mutex;
    pthread_mutex_t stop_mutex;
    int sim_status;
} simulation_data;


typedef struct coder_info {
    unsigned long last_compilation_time;
    int thread_id;
    unsigned long compile_count;
} coder_info;


typedef struct dongle {
    int in_use;
    coder_info queue[2];
    int queue_count;
    unsigned long last_used_time;
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
unsigned long get_time(struct timeval start_time);
void safe_print(thread_data *data, char *text);
int safe_sleep(thread_data *coder, unsigned long ms);
void swap(coder_info *a, coder_info *b);

void *routine(void *coders);
void *monitor(void *arg);
void wake_all(thread_data *coders, unsigned long nb_coders);

int wait_dongle(thread_data *coder, dongle *dongle);
void release_dongle(thread_data *coder, dongle *dongle);
void add_to_queue(thread_data *coder, dongle *dongle);


#endif