#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

void print_data( bool write_as_null, off_t volume, char data ) {
    if (( int ) data == -1 ) {
        if ( write_as_null )
            printf( "<NULL>\t %ld\n", volume );
        else
            printf( "0\t\t %ld\n", volume );
    } else if (( int ) data > 32 && ( int ) data < 127 )
        printf( "%c\t %ld\n", data, volume );
    else
        printf( "%d\t %ld\n", ( int ) data, volume );
}

int main( int argc, char **argv ) {
//    off_t count_printed_data = 0;
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
    for ( ; offset_current < size; ) {
        off_t offset_data = lseek( plik, offset_current, SEEK_DATA );
        if ( offset_data == -1 )    //skonczyly sie dane
            offset_data = size;

        if ( offset_data != 0 ) {
            print_data( write_as_null, offset_data - offset_current, ( char ) -1 );
//            count_printed_data += offset_data - offset_current;

            if ( offset_data == size ) {
//                printf( "Wypisano %ld z %ld", count_printed_data, size );
                return 0;
            }
        }

        offset_current = offset_data;
        off_t offset_hole = lseek( plik, offset_current, SEEK_HOLE );

        if ( offset_data != offset_hole ) {
            off_t data_block_size = offset_hole - offset_current;
            char *data = ( char * ) calloc( data_block_size, sizeof( char ));
            lseek( plik, offset_current, SEEK_SET );
            int red = read( plik, data, data_block_size );
            if ( red == -1 )
                perror( "Error during read" );

            off_t count_data = 1;
            for ( int i = 1; i < data_block_size; i++ ) {
                if ( data[ i ] == data[ i - 1 ] )
                    count_data++;
                else {
                    print_data( write_as_null, count_data, data[ i - 1 ] );
//                    count_printed_data += count_data;
                    count_data = 1;
                }
            }
            print_data( write_as_null, count_data, data[ data_block_size - 1 ] );
//            count_printed_data += count_data;

            memset( data, 0, data_block_size );
        }
        offset_current = offset_hole;
    }

//    printf( "Wypisano %ld z %ld", count_printed_data, size );
    return 0;
}