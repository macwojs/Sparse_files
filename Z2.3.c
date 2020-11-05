#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>


int main( int argc, char **argv ) {

    off_t count_printed_data = 0;

    time_t t;
    srand(( unsigned ) time( &t ));

    int option;
    bool write_as_null = 1;
    char *file_path;

    while (( option = getopt( argc, argv, "!" )) != -1 ) {
        switch ( option ) {
            case '!':
                write_as_null = 0;
                break;
            default:
                printf( "Wrong input option" );
                break;
        }
    }

    // open file
    file_path = argv[ optind ];
    int plik = open( file_path, O_RDONLY );

    // get file size
    struct stat buf;
    fstat( plik, &buf );
    off_t size = buf.st_size;

    off_t offset_current = 0;
    for ( offset_current = 0; offset_current < size; ) {
        off_t offset_data = lseek( plik, offset_current, SEEK_DATA );
        if ( offset_data == -1 ) {   //skonczyly sie dane
            offset_data = size;
        }

        if ( offset_data != 0 ) {
            if ( write_as_null )
                printf( "<NULL>\t %ld\n", offset_data - offset_current );
            else
                printf( "0\t\t %ld\n", offset_data - offset_current );

            count_printed_data +=offset_data - offset_current;

            if ( offset_data == size ){
                printf("Wypisano %ld z %ld", count_printed_data, size);
                return 0;
            }
        }

        offset_current = offset_data;
        off_t offset_hole = lseek( plik, offset_current, SEEK_HOLE );

        if ( offset_data != offset_hole ) {
            off_t data_block_size = offset_hole - offset_current;
            char *data = ( char * ) calloc( data_block_size, sizeof( char ));
            lseek( plik, 0, SEEK_SET );
            int red = read( plik, data, data_block_size );
            if ( red == -1 )
                perror( "Error during read" );

            off_t count_data = 1;
            for ( int i = 1; i < data_block_size; i++ ) {
                if ( data[ i ] == data[ i - 1 ] )
                    count_data++;
                else {
                    printf( "%c\t\t %ld\n", data[ i - 1 ], count_data );
                    count_printed_data += count_data;
                    count_data = 1;
                }
            }
            printf( "%c\t\t %ld\n", data[ data_block_size - 1 ], count_data );
            count_printed_data += count_data;
        }
        offset_current = offset_hole;
    }

    printf("Wypisano %ld z %ld", count_printed_data, size);
    return 0;
}