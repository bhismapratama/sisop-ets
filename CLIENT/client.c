#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

int main(int argc, char *argv[]) {

    // Mem-parse argumen
    char *alamat_ip;
    char *port;
    int perintah;
    
    while ((perintah = getopt(argc, argv, "i:p:")) != -1) {
        switch (perintah) {
            case 'i':
                alamat_ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (alamat_ip == NULL || port == NULL) exit(EXIT_FAILURE);

    int kode_keluar = 0;

    rpc_client *status = rpc_inisialisasi_client("::1", 3000);
    if (status == NULL) {
        exit(EXIT_FAILURE);
    }

    rpc_pengelola *pengelola_tambah2 = rpc_cari(status, "tambah2");
    if (pengelola_tambah2 == NULL) {
        fprintf(stderr, "ERROR: Fungsi tambah2 tidak ada\n");
        kode_keluar = 1;
        goto bersihkan;
    }

    for (int i = 0; i < 2; i++) {
        /* Persiapkan permintaan */
        char operand_kiri = i;
        char operand_kanan = 100;
        rpc_data data_permintaan = {
            .data1 = operand_kiri, .data2_len = 1, .data2 = &operand_kanan};

        /* Panggil dan terima respons */
        rpc_data *data_respons = rpc_panggil(status, pengelola_tambah2, &data_permintaan);
        if (data_respons == NULL) {
            fprintf(stderr, "Panggilan fungsi tambah2 gagal\n");
            kode_keluar = 1;
            goto bersihkan;
        }

        /* Interpretasi respons */
        assert(data_respons->data2_len == 0);
        assert(data_respons->data2 == NULL);
        printf("Hasil penambahan %d dan %d: %d\n", operand_kiri, operand_kanan,
            data_respons->data1);
        rpc_bebas_data(data_respons);
    }

bersihkan:
    if (pengelola_tambah2 != NULL) {
        free(pengelola_tambah2);
    }

    rpc_tutup_client(status);
    status = NULL;

    return kode_keluar;
}
