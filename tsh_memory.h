#ifndef TSH_MEMORY_H
#define TSH_MEMORY_H

#define BUFSIZE 512
typedef struct tsh_memory{
    char *PATH;//the current path
    int path_descriptor;//we keep update the current path also in a file, so it's remain the same for all the processus
    int tar_descriptor;//so we can get information about the main file (the .tar)
    char **comand;//the command that we are typing
    int tail_comand;
}tsh_memory;

extern tsh_memory * instanciate_tsh_memory(char *path_file_name, char *tar_file_name);
extern void update_path(tsh_memory * state);
extern void free_tsh_memory(tsh_memory *state);
extern void getCommand_plus_fd(char **command_typed, tsh_memory *memory);
#endif
