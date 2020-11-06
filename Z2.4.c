#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>


int main( int argc, char **argv ) {

    int option;
    char *file_path;
    while (( option = getopt( argc, argv, "s:f:" )) != -1 ) {
        if ( option == 's' )
            file_path = optarg;
        else {
            printf( "Wrong input option" );
            return -1;
        }
    }

    if ( argc < 4 ) {
        printf( "Need to provide more data" );
        return -1;
    }

    long int exp_size_before = 0;
    long int exp_size_after = 0;

    //input string split
    char *input_str = argv[ optind ];
    char *direction = strtok( input_str, ":" );
    char *size = strtok(NULL, ":" );
    if ( !strcmp( direction, "ambo" )) {
        exp_size_before = strtol( size, NULL, 0 );
        exp_size_after = strtol( size, NULL, 0 );
    } else if ( !strcmp( direction, "ante" ))
        exp_size_before = strtol( size, NULL, 0 );
    else if (( !strcmp( direction, "post" )))
        exp_size_after = strtol( size, NULL, 0 );
    else {
        printf( "Wrong argument" );
        return -1;
    }

    if (( !strcmp( direction, "ante" ) || !strcmp( direction, "post" )) && argc == 5 ) {
        optind++;
        char *input_str2 = argv[ optind ];
        char *direction2 = strtok( input_str2, ":" );
        char *size2 = strtok(NULL, ":" );
        if ( !strcmp( direction2, "ante" ))
            exp_size_before = strtol( size2, NULL, 0 );
        else if (( !strcmp( direction2, "post" )))
            exp_size_after = strtol( size2, NULL, 0 );
        else {
            printf( "Wrong argument" );
            return -1;
        }
    }

    return 0;
}