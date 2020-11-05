#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv) {

    int option;
    int size;
    char *file_path;
    while (optind < argc) {
        if ((option = getopt(argc, argv, "r:")) != -1) {
            switch (option) {
                case 'r':
                    size = atoi(optarg);
                    break;
                default:
                    break;
            }
        } else {
            file_path = argv[optind];
            optind++;
        }
    }

    int check = open(file_path, O_RDONLY);
    if (check != -1) {
        perror("File exist");
        close(check);
        return -1;
    }

    int f = open(file_path, O_CREAT|O_WRONLY, 0777);
//    lseek(f, size*8*1024, SEEK_CUR);
//    int numWritten = write(f, "", 1);
//    if (numWritten == -1)
//        perror("write");
//    printf("%s: wrote %ld bytes\n", "0", (long) numWritten);
    if (ftruncate(f, size*8*1024 ) == -1)
        perror("Error with allocate file");
    close(f);

    return 0;
}




