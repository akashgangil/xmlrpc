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
  xmlrpc_int32 server_id, status;

  /* Initialize our error environment variable */
  xmlrpc_env_init(&env);
  
  if (faultP->fault_occurred)
    printf("The RPC failed.  %s\n", faultP->fault_string);
  else {
    /* Get our sum and print it out. */
    xmlrpc_decompose_value(&env, resultP, "i", &status);
    die_if_fault_occurred(&env);
    printf("The status is %d\n", status);
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
  const int semantic = 1;
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
  xmlrpc_client_call_asynch(serverUrl, semantic, methodName, handle_status_response,
                            NULL, "(i)", &server_id);
  die_if_fault_occurred(&env);

  printf("RPCs all requested.  Waiting for & handling responses...\n");

  /* Wait for all RPCs to be done.  With some transports, this is also
     what causes them to go.
   */
  xmlrpc_client_event_loop_finish_asynch();


  printf("All RPCs finished.\n");

  xmlrpc_client_cleanup();

  xmlrpc_client_teardown_global_const();

  return 0;
}
