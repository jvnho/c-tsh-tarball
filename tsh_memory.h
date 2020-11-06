#ifndef TSH_MEMORY_H
#define TSH_MEMORY_H

#define BUFSIZE 512
#define MAX_COMMAND 512
typedef struct tsh_memory{
    char FAKE_PATH[BUFSIZE];//the current path from the tar
    char tar_name[BUFSIZE];
    char tar_descriptor[BUFSIZE];//so we can get information about the main file (the .tar)
    char comand[MAX_COMMAND];//the command that we are typing
    char REAL_PATH[BUFSIZE];//the buff when we read the getcwd
    int exit;
}tsh_memory;

extern int instanciate_tsh_memory(char *tar_file_name, tsh_memory *result);
extern tsh_memory * create_memory();
extern char * getPath(tsh_memory *state);
extern void free_tsh_memory(tsh_memory *state);
extern int in_a_tar(tsh_memory *state);
extern void resetBuffer(tsh_memory *memory);
extern void exit2(tsh_memory *state);
#endif
