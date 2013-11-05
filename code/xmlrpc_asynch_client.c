/* A simple asynchronous XML-RPC client written in C, as an example of
   Xmlrpc-c asynchronous RPC facilities.  This is the same as the
   simpler synchronous client xmlprc_sample_add_client.c, except that
   it adds 3 different pairs of numbers with the summation RPCs going on
   simultaneously.

   Use this with xmlrpc_sample_add_server.  Note that that server
   intentionally takes extra time to add 1 to anything, so you can see
   our 5+1 RPC finish after our 5+0 and 5+2 RPCs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "config.h"  /* information about this build environment */

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

int client_busy_ctr=0;
int client_idle_ctr=0;
int client_most_busy_ctr=0;
int client_most_idle_ctr=0;
int client_rpc_failure_ctr = 0;
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

}

int
main(int           const argc,
    const char ** const argv) {

  const char * const serverUrl = "http://localhost:8080/RPC2";
  const char * const methodName = "status";

  xmlrpc_env env;
  xmlrpc_client * clientP;
  xmlrpc_int adder;
  const int semantic = 2;
  xmlrpc_int32 server_id;

  if (argc-1 > 0) {
    fprintf(stderr, "This program has no arguments\n");
    exit(1);
  }

  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);

  /* Required before any use of Xmlrpc-c client library: */
  xmlrpc_client_setup_global_const(&env);
  die_if_fault_occurred(&env);

  xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
  die_if_fault_occurred(&env);

  server_id = 1;

  int i = 0;

  for(i=0;i<120;i++) {
    xmlrpc_client_call_asynch(serverUrl, semantic, methodName, handle_status_response,
                              NULL, "(iii)", server_id, semantic, i);
    die_if_fault_occurred(&env);
  }


  /* Wait for all RPCs to be done.  With some transports, this is also
     what causes them to go.
   */
  xmlrpc_client_event_loop_finish_asynch();

  int total, failures;
  total = client_busy_ctr + client_idle_ctr + client_most_busy_ctr + client_most_idle_ctr + client_rpc_failure_ctr;

  if (total < 1000) {
    client_rpc_failure_ctr = i-total;
  }
  
  printf("%d|%d|%d|%d|%d|async\n", client_busy_ctr, client_idle_ctr, client_most_busy_ctr, client_most_idle_ctr, client_rpc_failure_ctr);

  xmlrpc_client_cleanup();

  xmlrpc_client_teardown_global_const();

  return 0;
}
