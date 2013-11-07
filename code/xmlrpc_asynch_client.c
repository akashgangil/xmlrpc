#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "config.h"  /* information about this build environment */
#include "timer.h"

#define NAME "Xmlrpc-c Asynchronous Test Client"
#define VERSION "1.0"

#define BUSY 0
#define IDLE 1
#define MOST_BUSY 2
#define MOST_IDLE 3
#define RPC_FAILURE 4

#define ANY 0
#define MAJORITY 1
#define ALL 2

#define NUM_SERVERS 3
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

int client_busy_ctr=0;
int client_idle_ctr=0;
int client_most_busy_ctr=0;
int client_most_idle_ctr=0;
int client_rpc_failure_ctr = 0;

static pthread_mutex_t asynch_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
die_if_fault_occurred(xmlrpc_env * const envP) {
  if (envP->fault_occurred) {
    fprintf(stderr, "Something failed. %s (XML-RPC fault code %d)\n",
        envP->fault_string, envP->fault_code);
    exit(1);
  }
}

static void
handle_status_response(const char *   const serverUrl,
    const char *   const methodName,
    xmlrpc_value * const paramArrayP,
    void *         const user_data,
    xmlrpc_env *   const faultP,
    xmlrpc_value * const resultP) {

  xmlrpc_env env;
  xmlrpc_int32 server_id, status, semantic;

  static num_responses = 0;

  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);

  if (faultP->fault_occurred)
    client_rpc_failure_ctr++;
  else {
    /* Get our sum and print it out. */
    xmlrpc_decompose_value(&env, resultP, "i", &status);
    die_if_fault_occurred(&env);

    switch(status){
    case BUSY: client_busy_ctr++; break;
    case IDLE: client_idle_ctr++; break;
    case MOST_BUSY: client_most_busy_ctr++; break;
    case MOST_IDLE: client_most_idle_ctr++; break;
    default: break;
    }
  }

  num_responses++;
  if(num_responses == 120) {
    pthread_mutex_unlock(&asynch_mutex);
  }
}

int
main(int           const argc,
    const char ** const argv) {

  if(argc != 3) {
    printf("Usage: ./xmlrpc_asynch_client <semantic> <number of requests>\n");
    printf("Semantic Options: ANY 0, MAJORITY 1, ALL 2\n");
    return 1;
  }

  const char * const serverUrl = "http://localhost:8080/RPC2";
  const char * const methodName = "status";

  xmlrpc_env env;
  xmlrpc_client * clientP;
  xmlrpc_int adder;
  xmlrpc_int32 semantic = atoi(argv[1]);
  const int num_requests = atoi(argv[2]);
  xmlrpc_int32 server_id;


  pthread_mutex_lock(&asynch_mutex);

  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);

  /* Required before any use of Xmlrpc-c client library: */
  xmlrpc_client_setup_global_const(&env);
  die_if_fault_occurred(&env);

  xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
  die_if_fault_occurred(&env);

  server_id = 1;

  int i = 0;

  struct stopwatch_t* sw = stopwatch_create();
  stopwatch_init();
  stopwatch_start(sw);
  
  for(i=0;i<num_requests;i++) {
    xmlrpc_client_call_asynch(serverUrl, semantic, methodName,3, handle_status_response,
                              NULL, "(iii)", server_id, semantic, i);
    die_if_fault_occurred(&env);
    
    /* Wait for all RPCs to be done.  With some transports, this is also
       what causes them to go.
    */
    xmlrpc_client_event_loop_finish_asynch();
  }

  pthread_mutex_lock(&asynch_mutex);

  stopwatch_stop(sw);   

  
  int total, failures;
  total = client_busy_ctr + client_idle_ctr + client_most_busy_ctr + client_most_idle_ctr + client_rpc_failure_ctr;

  if (total < 1000) {
    client_rpc_failure_ctr = i-total;
  }


  switch(semantic) {
    case 0: printf("any|%d|%d|%d|%d|%d|async|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, stopwatch_elapsed(sw)/(NUM_SERVERS*num_requests)); break;
    case 1: printf("majority|%d|%d|%d|%d|%d|async|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, stopwatch_elapsed(sw)/(NUM_SERVERS*num_requests)); break;
    case 2: printf("all|%d|%d|%d|%d|%d|async|%Lg\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr, stopwatch_elapsed(sw)/(NUM_SERVERS*num_requests)); break;
    default: printf("Use 0|1|2 as arguments"); break;
  }
  
  stopwatch_destroy(sw);

  xmlrpc_client_cleanup();

  xmlrpc_client_teardown_global_const();

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

