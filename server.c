#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

rpc_data *tambahkan2_i8(rpc_data *);

int main(int argc, char *argv[]) {

    // parsning argumen dari baris perintah
    char *port;
    int perintah;

    // getopt
    while ((perintah = getopt(argc, argv, "p:")) != -1) {
        switch (perintah) {
            case 'p':
                port = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (port == NULL) exit(EXIT_FAILURE);

    rpc_server *status;

    status = rpc_init_server(3000);
    if (status == NULL) {
        fprintf(stderr, "Gagal menginisialisasi\n");
        exit(EXIT_FAILURE);
    }

    if (rpc_daftarkan(status, "tambahkan2", tambahkan2_i8) == -1) {
        fprintf(stderr, "Gagal mendaftarkan fungsi tambahkan2\n");
        exit(EXIT_FAILURE);
    }

    rpc_layani_semua(status);

    return 0;
}

rpc_data *tambahkan2_i8(rpc_data *masukan) {

    /* Periksa data2 */
    if (masukan->data2 == NULL || masukan->data2_len != 1) {
        return NULL;
    }

    /* Parse permintaan */
    char n1 = masukan->data1;
    char n2 = ((char *)masukan->data2)[0];

    /* Melakukan perhitungan */
    printf("tambahkan2: argumen %d dan %d\n", n1, n2);
    int hasil = n1 + n2;

    /* Persiapkan respons */
    rpc_data *keluaran = malloc(sizeof(rpc_data));
    assert(keluaran != NULL);
    keluaran->data1 = hasil;
    keluaran->data2_len = 0;
    keluaran->data2 = NULL;
    return keluaran;
}
