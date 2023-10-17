#include "rpc.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Fungsi Socket
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAKS_DATA1 8

struct server_rpc {
  int port;
  int jumlah_fungsi;
  char ** fungsi;
  rpc_pengelola * pengelola;
  int socket;
};

/*  Memperluas struktur data untuk mencakup lokasi, karena tidak dapat
    memodifikasi rpc.h */
typedef struct {
  rpc_data data;
  int lokasi;
}
rpc_data_lokasi;

typedef enum {
  CARI,
  PANGGIL
}
jenis_permintaan_rpc;

server_rpc *
  rpc_inisialisasi_server(int port) {
    server_rpc * server = malloc(sizeof(server_rpc));
    server -> port = port;
    server -> jumlah_fungsi = 0;
    server -> fungsi = NULL;
    server -> pengelola = NULL;

    // Siapkan socket
    server -> socket = socket(AF_INET6, SOCK_STREAM, 0);

    // Opsi penggunaan kembali
    int aktifkan = 1;
    if (setsockopt(server -> socket, SOL_SOCKET, SO_REUSEADDR, & aktifkan,
        sizeof(int)) == -1) {
      close(server -> socket);
      free(server -> fungsi);
      free(server -> pengelola);
      free(server);
      return NULL;
    }

    // Siapkan alamat
    struct sockaddr_in6 alamat;
    memset( & alamat, 0, sizeof(alamat));
    alamat.sin6_family = AF_INET6;
    alamat.sin6_addr = in6addr_any;
    alamat.sin6_port = htons(port);

    // Binding
    if (bind(server -> socket, (struct sockaddr * ) & alamat,
        sizeof(alamat)) == -1) {
      close(server -> socket);
      free(server -> fungsi);
      free(server -> pengelola);
      free(server);
      return NULL;
    }

    if (listen(server -> socket, SOMAXCONN) == -1) {
      close(server -> socket);
      free(server -> fungsi);
      free(server -> pengelola);
      free(server);
      return NULL;
    }

    return server;
  }

int
rpc_daftarkan(server_rpc * srv, char * nama, rpc_pengelola pengelola) {

  // Mengubah ukuran array secara dinamis untuk fungsi yang ditambahkan baru
  srv -> fungsi =
    (char ** ) realloc(srv -> fungsi,
      (srv -> jumlah_fungsi + 1) * sizeof(char * ));
  srv -> pengelola =
    (rpc_pengelola * ) realloc(srv -> pengelola,
      (srv -> jumlah_fungsi +
        1) * sizeof(rpc_pengelola));

  // Tambahkan fungsi/pengelola ke server
  srv -> fungsi[srv -> jumlah_fungsi] = strdup(nama);
  srv -> pengelola[srv -> jumlah_fungsi] = pengelola;
  srv -> jumlah_fungsi++;

  return 0;
}

// Cari apakah/dimana modul ada di server
int
rpc_cari_lokasi(server_rpc * srv, char * nama) {
  for (int i = 0; i < srv -> jumlah_fungsi; i++) {
    if (strcmp(srv -> fungsi[i], nama) == 0)
      return i;
  }

  return -1;
}

void
rpc_layani_semua(server_rpc * srv) {

  while (1) {

    // Deteksi pesan dari klien
    int klien = accept(srv -> socket, NULL, NULL);
    if (klien < 0)
      continue;

    // Deteksi jenis permintaan
    jenis_permintaan_rpc jenis_permintaan;
    if (recv(klien, & jenis_permintaan, sizeof(jenis_permintaan),
        0) == -1) {
      close(klien);
      continue;
    };

    if (jenis_permintaan == CARI) {

      // Perbarui nama modul dalam permintaan pencarian
      size_t panjang;
      if (recv(klien, & panjang, sizeof(panjang), 0) == -1) {
        close(klien);
        continue;
      };
      char * nama = malloc(panjang + 1);
      if (recv(klien, nama, panjang, 0) == -1) {
        close(klien);
        free(nama);
        continue;
      };
      nama[panjang] = '\0';

      // Kirim keberadaan modul ke klien
      int lokasi = rpc_cari_lokasi(srv, nama);
      if (send(klien, & lokasi, sizeof(lokasi), 0) ==
        -1) {
        close(klien);
        free(nama);
        continue;
      };

      free(nama);
    } else if (jenis_permintaan == PANGGIL) {

      // Terima lokasi pengelola dari klien
      rpc_data_lokasi permintaan;
      if (recv(klien, & permintaan, sizeof(permintaan), 0) == -1) {
        close(klien);
        continue;
      };

      // Terima format data dari klien
      if (recv(klien, & permintaan.data.data2_len,
          sizeof(permintaan.data.data2_len), 0) == -1) {
        close(klien);
        free(permintaan.data.data2);
        continue;
      };
      if (sizeof(permintaan.data.data1) > MAKS_DATA1) {
        close(klien);
        continue;
      }
      permintaan.data.data2 = malloc(permintaan.data.data2_len);
      if (recv(klien, permintaan.data.data2,
          permintaan.data.data2_len, 0) == -1) {
        close(klien);
        free(permintaan.data.data2);
        continue;
      };
      if (permintaan.lokasi < 0 ||
        permintaan.lokasi >= srv -> jumlah_fungsi) {
        close(klien);
        continue;
      }

      // Panggil pengelola
      rpc_pengelola pengelola = srv -> pengelola[permintaan.lokasi];
      rpc_data * respons = pengelola( & (permintaan.data));
      free(permintaan.data.data2);

      // Kirim konfirmasi ke klien
      if (respons != NULL) {
        if (send(klien, respons, sizeof( * respons), 0) ==
          -1) {
          close(klien);
          continue;
        };
        rpc_data_free(respons);
      }
    }

    close(klien);
  }
}

struct klien_rpc {
  char * ip;
  int port;
};

struct handle_rpc {
  int lokasi;
};

klien_rpc *
  rpc_inisialisasi_klien(char * alamat, int port) {
    klien_rpc * klien = malloc(sizeof(klien_rpc));
    klien -> ip = strdup(alamat);
    if (klien -> ip == NULL) {
      free(klien);
      return NULL;
    }
    klien -> port = port;
    return klien;
  }

handle_rpc *
  rpc_cari(klien_rpc * cl, char * nama) {

    // Buat socket
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      close(socket_fd);
      return NULL;
    }

    // Alamat server
    struct sockaddr_in6 alamat;
    memset( & alamat, 0, sizeof(alamat));
    alamat.sin6_family = AF_INET6;
    inet_pton(AF_INET6, cl -> ip, & (alamat.sin6_addr));
    alamat.sin6_port = htons(cl -> port);

    // Hubungkan ke server
    if (connect(socket_fd, (struct sockaddr * ) & alamat,
        sizeof(alamat)) < 0) {
      close(socket_fd);
      return NULL;
    }

    // Kirim jenis permintaan ke server
    jenis_permintaan_rpc jenis_permintaan = CARI;
    if (send(socket_fd, & jenis_permintaan, sizeof(jenis_permintaan),
        0) == -1) {
      close(socket_fd);
      return NULL;
    };

    // Kirim modul ke server
    size_t panjang = strlen(nama);
    if (send(socket_fd, & panjang, sizeof(panjang), 0) == -1) {
      close(socket_fd);
      return NULL;
    };
    if (send(socket_fd, nama, panjang, 0) == -1) {
      close(socket_fd);
      return NULL;
    };

    // Terima indeks modul
    int lokasi;
    if (recv(socket_fd, & lokasi, sizeof(lokasi), 0) ==
      -1) {
      close(socket_fd);
      return NULL;
    };
    close(socket_fd);
    if (lokasi == -1) {
      return NULL;
    }

    // Simpan indeks fungsi dalam handled, kembalikan handled
    handle_rpc * handled = malloc(sizeof(handle_rpc));
    handled -> lokasi = lokasi;
    return handled;
  }

rpc_data *
  rpc_panggil(klien_rpc * cl, handle_rpc * h, rpc_data *
    beban) {

    // Buat socket
    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      close(socket_fd);
      return NULL;
    }

    // Alamat server
    struct sockaddr_in6 alamat;
    memset( & alamat, 0, sizeof(alamat));
    alamat.sin6_family = AF_INET6;
    inet_pton(AF_INET6, cl -> ip, & (alamat.sin6_addr));
    alamat.sin6_port = htons(cl -> port);

    // Hubungkan ke server
    if (connect(socket_fd, (struct sockaddr * ) & alamat,
        sizeof(alamat)) < 0) {
      close(socket_fd);
      return NULL;
    }

    // Siapkan struktur data rpc
    rpc_data_lokasi permintaan;

    // Penanganan persyaratan Tugas 5, maks 8 byte
    if (sizeof(beban -> data1) > MAKS_DATA1) {
      close(socket_fd);
      return NULL;
    }
    permintaan.data = * beban;
    permintaan.lokasi = h -> lokasi;

    // Kirim jenis permintaan ke server
    jenis_permintaan_rpc jenis_permintaan = PANGGIL;
    if (send(socket_fd, & jenis_permintaan, sizeof(jenis_permintaan),
        0) == -1) {
      close(socket_fd);
      return NULL;
    };

    // Kirim bidang data yang benar ke server
    if (send(socket_fd, & permintaan, sizeof(permintaan), 0) == -1) {
      close(socket_fd);
      return NULL;
    };
    if (send(socket_fd, & beban -> data2_len,
        sizeof(beban -> data2_len), 0) == -1) {
      close(socket_fd);
      return NULL;
    };
    if (send(socket_fd, beban -> data2, beban -> data2_len, 0) ==
      -1) {
      close(socket_fd);
      return NULL;
    };

    // Dapatkan dan kembalikan respons
    rpc_data * respons = malloc(sizeof(rpc_data));
    if (recv(socket_fd, respons, sizeof( * respons), 0) ==
      -1) {
      close(socket_fd);
      free(respons);
      return NULL;
    };
    close(socket_fd);
    return respons;
  }

void
rpc_tutup_klien(klien_rpc * cl) {
  if (cl == NULL)
    return;
  free(cl -> ip);
  free(cl);
}

void
rpc_bebas_data(rpc_data * data) {
  if (data == NULL) {
    return;
  }
  if (data -> data2 != NULL) {
    free(data -> data2);
  }
  free(data);
}
