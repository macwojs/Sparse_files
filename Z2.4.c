#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

int find_ext( char *ext ) {
    int mult = 1;
    if ( !strcmp( ext, "bb" ))
        mult = 512;
    else if ( !strcmp( ext, "K" ))
        mult = 1024;
    else if(!strcmp( ext, "B" ))
        mult = 1;
    else{
        printf("Wrong number ext");
        exit(-2);
    }
    return mult;
}

void parse_direction(char *direction, char* size, long int *exp_size_after, long int *exp_size_before){
    char *ext;
    if ( !strcmp( direction, "ambo" )) {
        *exp_size_before = strtol( size, &ext, 0 );
        *exp_size_after = strtol( size, &ext, 0 );
        *exp_size_after *= find_ext(ext);
        *exp_size_before *= find_ext(ext);
    } else if ( !strcmp( direction, "ante" )){
        *exp_size_before = strtol( size, &ext, 0 );
        *exp_size_before *= find_ext(ext);
    }
    else if (( !strcmp( direction, "post" ))){
        *exp_size_after = strtol( size, &ext, 0 );
        *exp_size_after *= find_ext(ext);
    }
    else {
        printf( "Wrong argument" );
        exit(-3);
    }
}

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
    long int exp_size_after=0;
    long int *p_exp_size_before=&exp_size_before;
    long int *p_exp_size_after=&exp_size_after;

    //input string split
    char *ext;
    char *input_str = argv[ optind ];
    char *direction = strtok( input_str, ":" );
    char *size = strtok(NULL, ":" );
    parse_direction(direction, size, p_exp_size_after, p_exp_size_before);

    if (( !strcmp( direction, "ante" ) || !strcmp( direction, "post" )) && argc == 5 ) {
        optind++;
        char *input_str2 = argv[ optind ];
        char *direction2 = strtok( input_str2, ":" );
        char *size2 = strtok(NULL, ":" );
        parse_direction(direction2, size2, p_exp_size_after, p_exp_size_before);
    }


    return 0;
}