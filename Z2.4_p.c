#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

int file;

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

char find_char_after( off_t block_size, off_t offset ) {
    char *data = ( char * ) calloc( block_size, sizeof( char ));
    lseek( file, offset, SEEK_SET );
    int red = read( file, data, block_size );
    if ( red == -1 )
        perror( "Error during read" );

    for ( off_t i = block_size - 1; i >= 0; i-- ) {
        if (( int ) data[ i ] != 0 ) {
            return data[ i ];
        }
    }
    return '0';
}

char find_char_before( off_t block_size, off_t offset ) {
    char *data = ( char * ) calloc( block_size, sizeof( char ));
    lseek( file, offset, SEEK_SET );
    int red = read( file, data, block_size );
    if ( red == -1 )
        perror( "Error during read" );

    for ( off_t i = 0; i < block_size; i++ ) {
        if (( int ) data[ i ] != 0 ) {
            return data[ i ];
        }
    }
    return '0';
}

void write_data( off_t offset_data, long int size, char current_char ) {
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
    file = open( file_path, O_RDWR );
    struct stat buf;
    fstat( file, &buf );
    off_t file_size = buf.st_size;

    off_t offset_hole1;
    off_t offset_data1;
    off_t offset_hole2;
    off_t offset_data2;

    off_t offset_start = 0;
    for ( ; offset_start < file_size; ) {
        offset_hole1 = lseek( file, offset_start, SEEK_HOLE );
        if ( offset_hole1 == 0 ) { //jezeli na poczatku jest pusta przestrzen
            offset_data1 = lseek( file, offset_start, SEEK_DATA );
            off_t size_current_hole = offset_data1;
            off_t available_space_before = exp_size_before;
            if(size_current_hole<exp_size_before)
                available_space_before=size_current_hole;
            off_t size_current_data_block = lseek( file, offset_data1, SEEK_HOLE ) - offset_data1;
            char char_to_add = find_char_before( size_current_data_block, offset_data1 );
            write_data( offset_data1 - available_space_before, available_space_before, char_to_add );
            offset_start = offset_data1;
            continue;
        }

        offset_data1 = lseek( file, offset_start, SEEK_DATA );
        offset_hole1 = lseek( file, offset_data1, SEEK_HOLE );


        if(offset_hole1==file_size){//nie ma juz przestrzeni za tymi danymi
            break;
        }

        offset_data2 = lseek( file, offset_hole1, SEEK_DATA );

        if ( offset_data2 == -1 ) { //nie ma juz wiecej danych, a jest przestrzen na koncu
            off_t size_current_hole = file_size - offset_hole1;
            off_t available_space_after = exp_size_after;
            if(size_current_hole<exp_size_after)
                available_space_after=size_current_hole;
            off_t size_current_data_block = offset_hole1 - offset_data1;
            char char_to_add = find_char_after( size_current_data_block, offset_data1 );
            write_data( offset_hole1, available_space_after, char_to_add );
            break;
        }

        offset_hole2 = lseek( file, offset_data2, SEEK_HOLE );

        off_t size_current_hole = offset_data2 - offset_hole1;
        off_t available_space_after;
        off_t available_space_before;
        if ( size_current_hole < exp_size_total ) {
            available_space_after = ( off_t ) (( double ) size_current_hole * proportion_for_after );
            available_space_before = size_current_hole - available_space_after;
        } else {
            available_space_after = exp_size_after;
            available_space_before = exp_size_before;
        }

        //write after
        off_t size_current_data_block = offset_hole1 - offset_data1;
        char char_to_add = find_char_after( size_current_data_block, offset_data1 );
        write_data( offset_hole1, available_space_after, char_to_add );

        //write before
        size_current_data_block = offset_hole2 - offset_data2;
        char_to_add = find_char_before( size_current_data_block, offset_data2 );
        write_data( offset_data2 - available_space_before, available_space_before, char_to_add );

        offset_start = offset_data2;
    }

    return 0;
}
