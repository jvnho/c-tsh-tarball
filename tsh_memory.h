#ifndef TSH_MEMORY_H
#define TSH_MEMORY_H

#define BUFSIZE 512
typedef struct tsh_memory{
    char PATH[BUFSIZE];//the current path
    char * tar_descriptor;//so we can get information about the main file (the .tar)
    char *comand;//the command that we are typing
    int tail_comand;
}tsh_memory;

extern tsh_memory * instanciate_tsh_memory(char *tar_file_name);
extern void free_tsh_memory(tsh_memory *state);
#endif
