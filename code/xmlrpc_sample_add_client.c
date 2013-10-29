/* A simple synchronous XML-RPC client written in C, as an example of an
   Xmlrpc-c client.  This invokes the sample.add procedure that the Xmlrpc-c
   example xmlrpc_sample_add_server.c server provides.  I.e. it adds two
   numbers together, the hard way.

   This sends the RPC to the server running on the local system ("localhost"),
   HTTP Port 8080.
*/

#include <stdlib.h>
#include <stdio.h>

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "config.h"  /* information about this build environment */

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

static void
dieIfFaultOccurred (xmlrpc_env * const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}



int
main(int           const argc,
     const char ** const argv) {

    xmlrpc_env env;
    xmlrpc_value * resultP;
    xmlrpc_int32 status, server_id;
    
    const char * const serverUrl = "http://localhost:8080/RPC2";
    const char * const methodName = "status";
    const int semantic = ALL;

    if (argc-1 > 0) {
        fprintf(stderr, "This program has no arguments\n");
        exit(1);
    }

    /* Initialize our error-handling environment. */
    xmlrpc_env_init(&env);

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    dieIfFaultOccurred(&env);

    /* set the server id */
    server_id = 1;

    printf("Making XMLRPC call to server url '%s' method '%s' "
           "and server id is %d\n", serverUrl, methodName, server_id);

    /* Make the remote procedure call */
    resultP = xmlrpc_client_call(&env, serverUrl, semantic, methodName,
                                 "(i)", server_id);
    dieIfFaultOccurred(&env);

    /* Get our status and print it out. */
    xmlrpc_read_int(&env, resultP, &status);
    dieIfFaultOccurred(&env);
    printf("[CLIENT] The status is %d\n", status);

    /* Dispose of our result value. */
    xmlrpc_DECREF(resultP);

    /* Clean up our error-handling environment. */
    xmlrpc_env_clean(&env);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_client_cleanup();

    return 0;
}
