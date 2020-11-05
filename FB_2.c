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
int los(int lower, int upper)
{
    return (rand() %
            (upper - lower + 1)) + lower;
}
int main(int argc, char* argv[])
{
    long ll;
    char* endptr;
    srand(time(0));
    FILE* plik;
    char c;
    char s1[] = "tekst";
    char s2[] = "liczba";
    int fd;
    while((c = getopt(argc, argv, "sf"))!=-1)
    {
        switch(c)
        {
            case 's':
                if( access( argv[optind], F_OK ) != -1 )
                {
                    plik = fopen(argv[optind],"r+");
                    if (plik == NULL)
                    {
                        perror("Nie udalo sie otworzyc pliku");
                        return 1;
                    }
                }
                else
                {
                    printf("Error/file exists\n");
                    return 23;
                }
                break;
            case 'f':
                fd = fileno(plik);
                struct stat buf;
                fstat(fd, &buf);
                off_t size = buf.st_size;
                printf("%ld \n", size);
                int* array = (int*)calloc(size, sizeof(int));
                int il_do_wpisania = 0;
                if(!strcmp(argv[optind],s1))
                    for(int i=optind+1; i< argc; i++)
                    {
                        int tr = 1;
                        for(int j=0; argv[i][j]!='\0'; j++)
                        {
                            il_do_wpisania++;
                            if(il_do_wpisania>size)
                            {
                                printf("error//il_do_wpis\n");
                                return 23;
                            }
                            for(;tr;)
                            {
                                int x = los(0,size-1);
                                if(!array[x])
                                {
                                    array[x] = 1;
                                    fseek(plik,x, SEEK_SET);
                                    char c = argv[i][j];
                                    fwrite(&c,1,1,plik);
                                    break;
                                }
                            }
                        }

                    }
                else if(!strcmp(argv[optind],s2))
                    for(int i=optind+1; i< argc; i++)
                    {
                        int tr = 1;
                        ll = strtol(argv[i], &endptr, 0);
                        if( *endptr )
                        {
                            printf("Error. Wrong argument -d (%s) \n", argv[i]);
                            continue;
                        }
                        il_do_wpisania++;
                        if(il_do_wpisania>size)
                        {
                            printf("error//il_do_wpis\n");
                            return 23;
                        }
                        for(;tr;)
                        {
                            int x = los(0,size-1);
                            if(!array[x])
                            {
                                array[x] = 1;
                                fseek(plik,x, SEEK_SET);
                                char c = ll;
                                fwrite(&c,1,1,plik);
                                break;
                            }
                        }
                    }
                break;
            default:
                printf("Error/wrong par\n");

        }
    }
    fclose(plik);
    return 1;
}