#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

int find_ext( char *ext ) {
    int multi = 1;
    if ( !strcmp( ext, "bb" ))
        multi = 512;
    else if ( !strcmp( ext, "K" ))
        multi = 1024;
    else if ( !strcmp( ext, "B" ) || ( int ) ext[ 0 ] == 0 )
        multi = 1;
    else {
        printf( "Wrong number ext" );
        exit( -2 );
    }
    return multi;
}

void parse_direction( char *direction, char *size, long int *exp_size_after, long int *exp_size_before ) {
    char *ext;
    if ( !strcmp( direction, "ambo" )) {
        *exp_size_before = strtol( size, &ext, 0 );
        *exp_size_after = strtol( size, &ext, 0 );
        *exp_size_after *= find_ext( ext );
        *exp_size_before *= find_ext( ext );
    } else if ( !strcmp( direction, "ante" )) {
        *exp_size_before = strtol( size, &ext, 0 );
        *exp_size_before *= find_ext( ext );
    } else if (( !strcmp( direction, "post" ))) {
        *exp_size_after = strtol( size, &ext, 0 );
        *exp_size_after *= find_ext( ext );
    } else {
        printf( "Wrong argument" );
        exit( -3 );
    }
}

long int count_place( long int place, long int exp_size_total, long int exp_size,
                      double proportion ) {
    if ( place < exp_size_total )
        place = ( long int ) (( double ) place * proportion );
    else
        place = exp_size;
    return place;
}

void write_data( int file, off_t offset_data, long int size, char current_char ) {
    lseek( file, offset_data, SEEK_SET );
    for ( int i = 0; i < size; i++ ) {
        int numWritten = write( file, &current_char, 1 );
        if ( numWritten == -1 )
            perror( "write" );
    }
}

int main( int argc, char **argv ) {

    int option;
    char *file_path;
    while (( option = getopt( argc, argv, "s:" )) != -1 ) {
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
    long int *p_exp_size_before = &exp_size_before;
    long int *p_exp_size_after = &exp_size_after;

    //input string split
    char *input_str = argv[ optind ];
    char *direction = strtok( input_str, ":" );
    char *size = strtok(NULL, ":" );
    parse_direction( direction, size, p_exp_size_after, p_exp_size_before );

    if (( !strcmp( direction, "ante" ) || !strcmp( direction, "post" )) && argc == 5 ) {
        optind++;
        char *input_str2 = argv[ optind ];
        char *direction2 = strtok( input_str2, ":" );
        char *size2 = strtok(NULL, ":" );
        parse_direction( direction2, size2, p_exp_size_after, p_exp_size_before );
    }

    double proportion_for_before = 1;
    double proportion_for_after = 1;
    if ( exp_size_before != 0 && exp_size_after != 0 ) {
        proportion_for_before = ( double ) exp_size_before / (( double ) exp_size_after + ( double ) exp_size_before );
        proportion_for_after = 1 - proportion_for_before;
    }

    long int exp_size_total = exp_size_after + exp_size_before;

    //open file and get it's size
    int file = open( file_path, O_RDWR );
    struct stat buf;
    fstat( file, &buf );
    off_t file_size = buf.st_size;

    off_t offset_current = 0;
    off_t offset_hole;
    off_t offset_data;
    off_t offset_data_e[3];
    for ( int i = 0; i < 3; i++ )
        offset_data_e[ i ] = 0;

    char current_char;
    off_t size_current_data_block = 0;
    for ( ; size_current_data_block < file_size; ) {
        int index = 1;
        while ( index != 3 ) {
            offset_data = lseek( file, offset_current, SEEK_DATA );
            if ( offset_data == -1 ) { // skończyły sie dane
                offset_data_e[ 2 ] = file_size;
                break;
            }
            offset_hole = lseek( file, offset_data, SEEK_HOLE );
            size_current_data_block = offset_hole - offset_data;

            char *data = ( char * ) calloc( size_current_data_block, sizeof( char ));
            lseek( file, offset_data, SEEK_SET );
            int red = read( file, data, size_current_data_block );
            if ( red == -1 )
                perror( "Error during read" );

            for ( int i = 0; i < size_current_data_block; i++ ) {
                if (( int ) data[ i ] > 0 ) {
                    offset_data_e[ index ] = offset_data + i;
                    if ( index == 1 )
                        current_char = data[ i ];
//                    offset_current = offset_data + i;
                    index++;
                }

                if ( index == 3 ) {
                    offset_current = offset_data + i;
                    break;
                }
            }

            if ( index != 3 )
                offset_current = offset_hole;

//            free(data);
        }

        //TODO dopisywanie danych
        long int place_before = offset_data_e[ 1 ] - offset_data_e[ 0 ];
        long int place_after = offset_data_e[ 2 ] - offset_data_e[ 1 ];

        if ( offset_data_e[ 0 ] == 0 ) {
            if ( place_before > exp_size_before )
                place_before = exp_size_before;
            place_after = count_place( place_after, exp_size_total, exp_size_after, proportion_for_after );
            write_data( file, offset_data_e[ 1 ] - place_before, place_before, current_char ); //before
            write_data( file, offset_data_e[ 1 ] + 1, place_after, current_char ); //after

        } else if ( offset_data_e[ 2 ] == file_size ) {
            place_before = count_place( place_before, exp_size_total, exp_size_before, proportion_for_before );
            if ( place_after > exp_size_after )
                place_after = exp_size_after;
            write_data( file, offset_data_e[ 1 ] - place_before, place_before, current_char ); //before
            write_data( file, offset_data_e[ 1 ] + 1, place_after, current_char ); //after

        } else {
            place_before = count_place( place_before, exp_size_total, exp_size_before, proportion_for_before );
            place_after = count_place( place_after, exp_size_total, exp_size_after, proportion_for_after );
            write_data( file, offset_data_e[ 1 ] - place_before, place_before, current_char ); //before
            write_data( file, offset_data_e[ 1 ] + 1, place_after, current_char ); //after
        }

        if ( offset_data_e[ 2 ] == file_size )
            break;

        //przesuwamy dane
        offset_data_e[ 0 ] = offset_data_e[ 1 ];
        offset_data_e[ 1 ] = offset_data_e[ 2 ];
        offset_current = offset_data_e[ 1 ];

    }

    return 0;
}