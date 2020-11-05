#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>

long long int rand_position( long long int max_size, int8_t *tab ) {
    long long int random = ( rand() % max_size );
    while ( tab[ random ] != -1 )
        random = ( rand() % max_size );
    printf( "Pozycja do zapisania: %lld\n", random );
    return random;
}

void write_to_file( int *f, long long int pos, int8_t *data ) {
    int a = lseek( *f, pos, SEEK_SET );
    int numWritten = write( *f, data, 1 );
    if ( numWritten == -1 )
        perror( "write" );
    printf( "%s: wrote %ld bytes\n", "0", ( long ) numWritten );
}

int main( int argc, char **argv ) {

    time_t t;
    srand(( unsigned ) time( &t ));

    int option;
    bool format_is_number;
    char *file_path;
    char *str;

    while (( option = getopt( argc, argv, "s:f:" )) != -1 ) {
        switch ( option ) {
            case 's':
                file_path = optarg;
                break;
            case 'f':
                str = optarg;
                if ( strcmp( "tekst", str ) == 0 )
                    format_is_number = false;
                else if ( strcmp( "liczba", str ) == 0 )
                    format_is_number = true;
                else {
                    perror( "Wrong format on input" );
                    return -1;
                }
                break;
            default:
                break;
        }
    }

    int f = open( file_path, O_WRONLY | O_NONBLOCK );
    struct stat buf;
    fstat(f, &buf);
    off_t file_size = buf.st_size;

    long long int current_size = 0;
    int8_t input_data[file_size];
    for ( long long int i = 0; i < file_size; i++ )
        input_data[ i ] = -1;

    char *input_str;
    long long int current_pos = 0;

    while ( optind < argc ) {
        if ( format_is_number ) {
            if ( current_size < file_size ) {
                current_pos = rand_position( file_size, input_data );
                input_data[ current_pos ] = strtol( argv[ optind ], NULL, 0 );
                write_to_file( &f, current_pos, &input_data[ current_pos ] );
                current_size++;
            } else {
                perror( "Too much data" );
                return -2;
            }
        } else {
            input_str = argv[ optind ];
            for ( int i = 0; i < strlen( input_str ); i++ ) {
                if ( current_size < file_size ) {
                    current_pos = rand_position( file_size, input_data );
                    input_data[ current_pos ] = ( int ) input_str[ i ];
                    write_to_file( &f, current_pos, &input_data[ current_pos ] );
                    current_size++;
                } else {
                    perror( "Too much data" );
                    return -2;
                }
            }
        }
        optind++;
    }

    close(f);
}



