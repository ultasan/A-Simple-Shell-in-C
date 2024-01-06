#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


char *cd(char *PATH, char *token){
    int counter = 0;
    int i =0;
    chdir(PATH);
    int err = chdir(token);
    perror("");
    if(err<0){
        return PATH;
    }
    if(token != ".."){
        strcat(PATH, "/");
        strcat(PATH, token);
    }
    if(token == ".."){
        for(i=0;i<100;i++){
            if(PATH[i] == '/'){
                PATH[i] = '\0';
            }
            chdir("..");
        }
    }
    printf("\nPATH: %s\n", PATH);
    for(i=0;i<100;i++){
        if(PATH[i] == '/'){
            counter++;
        }
    }
    if(counter > 1){
        for(i=0;i<counter;i++){
            chdir("..");
        }
    }
    return PATH;
}

char *CommandSearch(char *command, char* PATH){
    int i = 0;
    int j = 0;
    int fields[2];
    int right = 1;
    char pathname[100];
    long max = pathconf(pathname, _PC_NAME_MAX);
    char arguements[10][max];
    char *cmd;
    char *token;
    char *filename = (char*)malloc(max*sizeof(char));
    chdir("/usr");
    token = strtok(command, " ");
    cmd = token;
    token = strtok(NULL, " ");
    if(strcmp(cmd,"cd") == 0){
        printf("\n\n\nToken: %s\n\n", token);
        PATH = cd(PATH, token);
        return PATH;
    }
    char *argv[500] = {cmd};
    
    i = 1;
    fflush(stdout);
    

    while(token != NULL){
        argv[i] = token;
        i++;
        token = strtok(NULL, " ");
        if(token == NULL){
            break;
        }
    }
    fflush(stdout);
    DIR *directory = opendir("bin");

    filename = readdir(directory)->d_name;
    // FINDING RIGHT FILE
    // find out all possible dirrectories
    while(filename != NULL){
        if(strcmp(filename,cmd) == 0){
            right = 0;
            break;
        }        
        filename = readdir(directory)->d_name;
    }
    if(right != 0){
        printf("Command '%s' not found!", cmd);
    }
    chdir("bin");
    getcwd(pathname,100);
    // READ THE FILE AND EXECUTE THE COMMAND
    

    
    char *env[] = {NULL};
    chdir(PATH);

    pipe(fields);
    int pid = fork();
    switch(pid){
        case -1:
            perror("fork");
            exit(1);
        case 0:
            if(close(fields[0] == -1)){
                perror("error closing read-end of child:");
                exit(1);
            }
            close(fields[0]);
            execvp(cmd, argv);  //syscall, libc has simpler wrappers (man exec)
            perror("execvp");
            close(fields[1]);
            break;
        default:
            close(fields[1]);
            char output[10000];
            wait(NULL);
            while(read(fields[0], output, 1) != 0){
                fprintf(stdout,"%s", output);
            }
    }
    kill(pid, SIGKILL);
    closedir(directory);
    filename = NULL;
    return PATH;
}



int main(){    
    char *PATH = (char*)malloc(100*sizeof(char));
    long max = pathconf(PATH, _PC_NAME_MAX);
    int i = 0;
    int terminate = 1;
    char *command = (char*)malloc(max*sizeof(char) + 10*sizeof(char));
    int j = 0;
    int counter = 0;
    char user[100];
    char hostname[100];
    getcwd(PATH, 100);
    for(i=0;i<100;i++){
        if(PATH[i] == '/'){
            counter++;
        }
    }
    if(counter > 1){
        for(i=0;i<counter;i++){
            chdir("..");
        }
    }

    FILE *hostfile = fopen("etc/hostname", "r");
    fscanf(hostfile,"%s", hostname);
    fclose(hostfile);
    counter = 0;
    i = 0;

    while(terminate == 1){
        fflush(stdin);
        fflush(stdout);
        printf("@%s:~%s$",hostname, PATH);
        fgets(command, 100, stdin);
        fflush(stdin);
        int length = strlen(command);
        command[length-1] = '\0';
        PATH = CommandSearch(command, PATH);
        i++;
    }

    free(command);
    free(PATH);
}
