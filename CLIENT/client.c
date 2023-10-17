#include "file_transfer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void unggah_berkas(char *nama_berkas, char *konten) {
    FILE *berkas = fopen(nama_berkas, "w");
    if (berkas == NULL) {
        perror("salah buka berkas");
        exit(1);
    }
    fprintf(berkas, "%s", konten);
    fclose(berkas);
}

void *unduh_dan_unggah(void *arg) {
    CLIENT *cl;
    char *server = "10.0.2.15:8000"; // coba ganti server IP teman

    cl = clnt_create(server, FILE_TRANSFER_PROG, FILE_TRANSFER_VERS, "udp");
    if (cl == NULL) {
        clnt_pcreateerror(server);
        exit(1);
    }

    char *nama_berkas = (char *)arg;
    char *konten = NULL;

    // Menggunakan RPC 
    if (unduh_1(&nama_berkas, &konten, cl) != RPC_SUCCESS || konten == NULL) {
        clnt_perror(cl, server);
        exit(1);
    }

    // Simpan berkas yang diunduh
    unggah_berkas(nama_berkas, konten);

    clnt_destroy(cl);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "yeay: %s <nama_berkas>\n", argv[0]);
        exit(1);
    }

    pthread_t thread;
    char *nama_berkas = argv[1];

    if (pthread_create(&thread, NULL, unduh_dan_unggah, (void *)nama_berkas)) {
        fprintf(stderr, "gabisa buat thread\n");
        exit(1);
    }

    pthread_join(thread, NULL);

    return 0;
}
