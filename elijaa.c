#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>

static xmlrpc_value* sapaElijaa(xmlrpc_env *env, xmlrpc_value *param_array, void *user_data) {
    const char *pesan = "halo elijaa";
    return xmlrpc_build_value(env, "s", pesan);
}

int main(int argc, char *argv[]) {
    xmlrpc_env env;
    xmlrpc_server_abyss_parms serverparm;
    const char * const host = "localhost";
    const unsigned int port = 8000;
    xmlrpc_registry *registryP;
    
    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);
    xmlrpc_registry_add_method(&env, registryP, NULL, "sapaElijaa", &sapaElijaa, NULL);

    serverparm.config_file_name = NULL;
    serverparm.registryP = registryP;
    serverparm.port_number = port;
    serverparm.log_file_name = NULL;
    serverparm.log_level = 0;

    printf("Starting XML-RPC server...\n");

    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

    /* Kode di bawah ini tidak akan pernah dieksekusi karena xmlrpc_server_abyss() tidak pernah kembali, kecuali ada kesalahan. */

    fprintf(stderr, "xmlrpc_server_abyss() returned unexpectedly.\n");
    exit(1);

    return 0;
}
