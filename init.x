program FILE_TRANSFER_PROG {
    version FILE_TRANSFER_VERS {
        int UPLOAD(char *, string) = 1;
        string DOWNLOAD(char *) = 2;
    } = 1;
} = 0x31234567;