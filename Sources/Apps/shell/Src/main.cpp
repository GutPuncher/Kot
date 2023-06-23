#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#include <kot/sys.h>

#define MAX_ARGS    255
#define MAX_CHAR    1024

char Input[MAX_CHAR];
char* Args[MAX_ARGS];

void GetInput() {
    // reset
    fflush(stdout);
    memset(Input, '\0', MAX_CHAR);

    fgets(Input, MAX_CHAR, stdin);

    Args[0] = strtok(Input, " \n\t");

    uint8_t i = 1;
    while((Args[i] = strtok(NULL, " \n\t")) != NULL && i < MAX_ARGS) {
        if(Args[i][0] == '$')
            Args[i] = getenv(Args[i] + 1);
        
        i++;
    }
}

int main(int argc, char* argv[], char* envp[]) {
    char* Prefix = "d1:/bin/";
    
    while(true) {
        // prompt
        char CommandInit[PATH_MAX + sizeof("$ ")];

        getcwd(CommandInit, PATH_MAX);

        strcat(CommandInit, "$ "); 

        printf(CommandInit);

        GetInput();

        if(!Args[0]) continue;

        pid_t pid = fork();

        if(pid == 0){
            size_t PathSize = strlen(Prefix) + strlen(Args[0]) + 1;
            char* Path = (char*)malloc(PathSize);
            Path[0] = '\0';

            strcat(Path, Prefix);
            strcat(Path, Args[0]);
            if(execvp(Path, Args) == -1){
                printf("Unknow command: %s\n", Args[0]);
                break;
            }
        }else{
            int Status;
            wait(&Status);
        }
    }
    
    return EXIT_SUCCESS;
}