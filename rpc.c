#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <curl/curl.h>

struct HasilRPC {
    int status;
    char keluaran[4096];
};

void* utas_anak(void* arg) {
    struct HasilRPC* hasil = (struct HasilRPC*)arg;

    char alamat_ip[16];  
    strcpy(alamat_ip, "10.0.2.15");

    char url[256];
    sprintf(url, "http://%s/api/upload", alamat_ip);

    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        curl_mime *form = NULL;
        curl_mimepart *field = NULL;

        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "file");
        curl_mime_filedata(field, "file_to_send.txt");

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_mime_free(form);
    }

    curl_global_cleanup();

    return NULL;
}

int main() {
    struct HasilRPC hasil;

    pthread_t tid;
    int err = pthread_create(&tid, NULL, utas_anak, &hasil);

    if (err != 0) {
        perror("pthread_create");
        return 1;
    }

    printf("Menunggu utas anak selesai...\n");
    pthread_join(tid, NULL);
    printf("Utas anak selesai.\n");

    if (hasil.status == 0) {
        printf("Hasil:\n%s\n", hasil.keluaran);
    } else {
        printf("Kesalahan selama panggilan RPC.\n");
    }

    return 0;
}
