#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <math.h>  
#include <fcntl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

// #ifndef SEEK_DATA
// #define SEEK_DATA 3
// #endif
//
// #ifndef SEEK_HOLE
// #define SEEK_HOLE 4
// #endif

int main(int argc, char* argv[])
{
    int plik;
    char c;
    int opcj = 0;
    while((c = getopt(argc, argv, "!"))!=-1)
    {
        switch(c)
        {
            case '!':
            opcj = 1;
            break;
 
            default:
            printf("Error/wrong par\n");
        }
    }
    plik = open(argv[optind],O_RDONLY);
    if (plik ==-1) 
    { 
        printf("Error Number % d\n", errno);  
        perror("Program"); 
        exit(1);                 
    } 
    struct stat buf;
    fstat(plik, &buf);
    off_t size = buf.st_size;
    int suma = 0;
    char nl = 48;
for(int i=0; i<size; )
    {
        int s = lseek(plik,i, SEEK_DATA);
        if(s==-1) //nie ma juz danych, wiec pozostale nulle do konca pliku wypisuje i koncze
        {
            if(!opcj) 
                printf("<NULL>\t %ld\n", size-i);
            else if(opcj)
                printf("%c \t %ld\n",nl, size-i);
            suma+=size-i;
            break;
        }
        int kon = lseek(plik, s, SEEK_HOLE);
        if(kon==-1)
        {
            perror("lseek hole");
            break;
        }
        int temp = kon - s;
        if(s-i>0)
        {
            if(!opcj)
                    printf("<NULL>\t %d\n", s-i);
            else if(opcj)
                    printf("%c \t %d\n",nl, s-i);
        }
        char* tab = (char*)calloc(temp,sizeof(char));
        lseek(plik, s, SEEK_SET);
        read(plik,tab,temp);
        for(int j=0; j<temp-1; j++)
        {
            int sum = 1;
            while(tab[j]==tab[j+1] && j<temp-1)
            {   
                sum++;
                j++;
            }
            if(tab[j]!='\0')
                printf("%c\t %d\n", tab[j], sum);
            if(!opcj && tab[j]=='\0')
                printf("<NULL>\t %d\n", sum);
            else if(opcj && tab[j]=='\0')
                printf("%c \t %d\n", tab[j], sum);
            suma += sum;
        }
        suma+=s;
        i = kon;        
    }

    printf("\nWypisane bajty: %d \t rozmiar pliku: %ld \n", suma, size);
    close(plik);
    return 1;
}