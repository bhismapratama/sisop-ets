#include "file_transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char* unduh_berkas(char *nama_berkas) {
    FILE *berkas = fopen(nama_berkas, "r");
    if (berkas == NULL) {
        perror("salah buka berkas");
        return NULL;
    }
    fseek(berkas, 0, SEEK_END);
    long ukuran_berkas = ftell(berkas);
    rewind(berkas);
    char *isi = (char *)malloc(ukuran_berkas);
    fread(isi, 1, ukuran_berkas, berkas);
    fclose(berkas);
    return isi;
}

bool_t unduh_svc(char *nama_berkas, char **isi, struct svc_req *rqstp) {
    *isi = unduh_berkas(nama_berkas);
    return TRUE;
}

void *jalankan_server(void *arg) {
    if (!svc_create(unduh_1, FILE_TRANSFER_PROG, FILE_TRANSFER_VERS, "netpath")) {
        fprintf(stderr, "gabis buat server\n");
        exit(1);
    }
    svc_run();
    fprintf(stderr, "svc_run \n");
    exit(1);
}

int main() {
    pthread_t thread;

    if (pthread_create(&thread, NULL, jalankan_server, NULL)) {
        fprintf(stderr, "gabisa buat thread\n");
        exit(1);
    }

    pthread_join(thread, NULL);

    return 0;
}
