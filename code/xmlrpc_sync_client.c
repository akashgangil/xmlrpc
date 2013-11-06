#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "config.h"  /* information about this build environment */
#include "timer.h"

#define NAME "Xmlrpc-c Test Client"
#define VERSION "1.0"

#define BUSY 0
#define IDLE 1
#define MOST_BUSY 2
#define MOST_IDLE 3
#define RPC_FAILURE 4

#define ANY 0
#define MAJORITY 1
#define ALL 2

#define NUM_REQUESTS 1000
#define NUM_SERVERS 3
#define TIMER_DESC "gettimeofday"
 
struct stopwatch_t
{
  struct timeval t_start_;
  struct timeval t_stop_;
  int is_running_;
};

struct stopwatch_t * stopwatch_create (void);
void stopwatch_destroy (struct stopwatch_t* T);
void stopwatch_init (void);
void stopwatch_start (struct stopwatch_t* T);
long double stopwatch_stop (struct stopwatch_t* T);
long double stopwatch_elapsed (struct stopwatch_t* T);

static void
dieIfFaultOccurred (xmlrpc_env * const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}


int client_busy_ctr=0;
int client_idle_ctr=0;
int client_most_busy_ctr=0;
int client_most_idle_ctr=0;
int client_rpc_failure_ctr = 0;

int
main(int           const argc,
        const char ** const argv) {

    if(argc != 3) {
      printf("Usage: ./xmlrpc_sync_client <semantic> <number of requests>\n");
      printf("Semantic Options: ANY 0 MAJORITY 1 ALL 2\n");
      return 1;
    }


    xmlrpc_env env;
    xmlrpc_value * resultP;
    xmlrpc_int32 status, server_id;

    const char * const serverUrl = "http://localhost:8080/RPC2";
    const char * const methodName = "status";
    xmlrpc_int32 semantic = atoi(argv[1]);

    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    dieIfFaultOccurred(&env);

    /* set the server id */
    server_id = 1;

    /* Make the remote procedure call */
    int i = 0;

    struct stopwatch_t* sw = stopwatch_create();
    stopwatch_init();
    stopwatch_start(sw);

    for(i=0;i<NUM_REQUESTS;i++){
        resultP = xmlrpc_client_call(&env, serverUrl, semantic, methodName,
                "(iii)", server_id, semantic, server_id);
        dieIfFaultOccurred(&env);
        xmlrpc_read_int(&env, resultP, &status);

        switch(status){
            case BUSY: client_busy_ctr++; break;
            case IDLE: client_idle_ctr++; break;
            case MOST_BUSY: client_most_busy_ctr++; break;
            case MOST_IDLE: client_most_idle_ctr++; break;
            default: break;
        }
    }

    stopwatch_stop(sw);   

    long double avg_time = stopwatch_elapsed(sw)/(NUM_SERVERS*NUM_REQUESTS);

    dieIfFaultOccurred(&env);
    switch(semantic) {
    case 0: printf("any|%d|%d|%d|%d|%d|sync|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, avg_time); break;
    case 1: printf("majority|%d|%d|%d|%d|%d|sync|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, avg_time); break;
    case 2: printf("all|%d|%d|%d|%d|%d|sync|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, avg_time); break;
    default: printf("Use 0|1|2 as arguments"); break;
    }
    
    stopwatch_destroy(sw);
    
    /* Dispose of our result value. */
    xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();

    return 0;
}

static long double elapsed (struct timeval start, struct timeval stop)
{
    return (long double)(stop.tv_sec - start.tv_sec)*1e3
        + (long double)(stop.tv_usec - start.tv_usec)*1e-3;
}

long double stopwatch_elapsed (struct stopwatch_t* T)
{
    long double dt = 0;
    if (T) {
        if (T->is_running_) {
            struct timeval stop;
            gettimeofday (&stop, 0);
            dt = elapsed (T->t_start_, stop);
        } else {
            dt = elapsed (T->t_start_, T->t_stop_);
        }
    }
    return dt;
}

void stopwatch_init (void)
{
    fflush (stderr);
}

void stopwatch_start (struct stopwatch_t* T)
{
    assert (T);
    T->is_running_ = 1;
    gettimeofday (&(T->t_start_), 0);
}

long double stopwatch_stop (struct stopwatch_t* T)
{
    long double dt = 0;
    if (T) {
        if (T->is_running_) {
            gettimeofday (&(T->t_stop_), 0);
            T->is_running_ = 0;
        }
        dt = stopwatch_elapsed (T);
    }
    return dt;
}


struct stopwatch_t * stopwatch_create (void)
{
    struct stopwatch_t* new_timer =
        (struct stopwatch_t *)malloc (sizeof (struct stopwatch_t));
    if (new_timer)
        memset (new_timer, 0, sizeof (struct stopwatch_t));
    return new_timer;
}

void stopwatch_destroy (struct stopwatch_t* T)
{
    if (T) {
        stopwatch_stop (T);
        free (T);
    }
}

