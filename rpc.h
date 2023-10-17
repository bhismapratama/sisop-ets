/* Header untuk sistem RPC */

#ifndef RPC_H
#define RPC_H

#include <stddef.h>

/* Status server */
typedef struct rpc_server rpc_server;
/* Status klien */
typedef struct rpc_client rpc_client;

/* datas permintaan */
typedef struct {
    int data1;
    size_t data2_len;
    void *data2;
} rpc_data;

/* fungsi remote */
typedef struct rpc_handle rpc_handle;

/* fungsi remote, yang mengambil rpc_data* sebagai input dan menghasilkan rpc_data* sebagai output */
typedef rpc_data *(*rpc_pengelola)(rpc_data *);

/* ---------------- */
/* Fungsi Server */
/* ---------------- */

/* Menginisialisasi status server */
/* MENGEMBALIKAN: rpc_server* jika sukses, NULL jika gagal */
rpc_server *rpc_inisialisasi_server(int port);

/* Mendaftarkan fungsi (pemetaan dari nama ke pengelola) */
/* MENGEMBALIKAN: -1 jika gagal */
int rpc_daftarkan(rpc_server *srv, char *nama, rpc_pengelola pengelola);

/* Mulai permintaan */
void rpc_layani_semua(rpc_server *srv);

/* ---------------- */
/* Fungsi Klien */
/* ---------------- */

/* Menginisialisasi status klien */
/* MENGEMBALIKAN: rpc_client* jika sukses, NULL jika gagal */
rpc_client *rpc_inisialisasi_klien(char *alamat, int port);

/* Mencari fungsi remote berdasarkan nama */
/* MENGEMBALIKAN: rpc_handle* jika sukses, NULL jika gagal */
/* rpc_handle* akan dibebaskan dengan satu panggilan ke free(3) */
rpc_handle *rpc_cari(rpc_client *cl, char *nama);

/* Memanggil fungsi remote */
/* MENGEMBALIKAN: rpc_data* jika sukses, NULL jika gagal */
rpc_data *rpc_panggil(rpc_client *cl, rpc_handle *h, rpc_data *beban);

/* Membersihkan status klien dan menutup klien */
void rpc_tutup_klien(rpc_client *cl);

/* ---------------- */
/* Fungsi Bersama */
/* ---------------- */

void rpc_bebas_data(rpc_data *data);

#endif
