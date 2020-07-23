#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>

void sh_loop(void);
char *sh_read_line(void);
char **sh_split_line(char* line);
int sh_execute(char **args);
int sh_launch(char **args);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

int main(int argc, char** argv){
    // Initialize - read and executes configuration files

    // Interpret - continuously read commands, often from stdin, and execute them
    sh_loop();

    // Terminate - execute shutdown commands and free up memory after previous commands are executed
    
    return EXIT_SUCCESS;
}

// Command loop
void sh_loop(void){
    char *line; // creating a NULL-terminated array of characters
    char **args; // creating a NULL-terminated array of pointers
    int status;
    // For a single command
    do{
        printf("$$ ");
        line = sh_read_line(); // Read - read command from stdin
        args = sh_split_line(line); // Parse - separate command string into arguments and action
        status = sh_execute(args); // Execute - execute the parsed command
        // Free memory of line and args
        free(line);
        free(args);
    }while(status);
}

// Reading command from stdin
#define BUFFERSIZE 1024
char *sh_read_line(void){
    int c; // c is the character. This variable is taken as int because EOF is an integer value.

    int size = BUFFERSIZE; // initializes size as 1024
    int position = 0; 
    char *buffer = malloc(sizeof(char) * size); // initializes a buffer of 1024 bits size
    // check allocation error
    if(!buffer){
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    // Take the input command
    while(1){
        c = getchar(); // Read a character
        // Put the character in the buffer
        if(c==EOF||c=="\n"){
            buffer[position] = '\0'; // If command ends, put NULL at the end of the buffer
            return buffer;
        }else{
            buffer[position] = c; // If commands doesn't end, put character in the buffer
        }
        position++; 

        // check if the buffer is full
        if(position >= size){
            size += BUFFERSIZE; // increase size by 1024
            buffer = realloc(buffer, size);
            //check reallocation error
            if(!buffer){
                fprintf(stderr, "sh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    } // close while
}

// Parse command
#define TOKENSIZE 64
#define DELIM " \t\n\r\a"
char **sh_split_line(char* line){
    char *token; // pointer to the token
    
    int size = TOKENSIZE; // initializes token size 
    int position = 0;
    char **tokens = malloc(sizeof(char*) * size); // initializes an array to store pointers to the tokens
    // check allocation error
    if(!tokens){
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    // breaks string into tokens using delimiter

    token = strtok(line, DELIM); // gets the first token

    // walk through other tokens
    while(token!=NULL){
        tokens[position] = token;
        position++;
        // check if the token array is full
        if(position >= size){
            size += TOKENSIZE; // increase size by 1024
            tokens = realloc(tokens, size);
            //check reallocation error
            if(!tokens){
                fprintf(stderr, "sh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM); // keep breaking the string
    }
    tokens[position] = NULL;
    return tokens;
}

// Execute programs

int sh_launch(char **args){
    pid_t pid, wpid; // initializes process ID variables
    int status;

    pid = fork(); // fork return 0 to child, and process ID of child to the parent
    if(pid == 0){
        // Child process
        if(execvp(args[0], args) == -1){ // execute a vector array(v) args, search for the new process in PATH(p), and alert if there is an error
            perror("sh");
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0){
        // Error in fork
        perror("sh");
    }else{
        // Parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED); // options = WUNTRACED
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

// Execute built-in commands

char *builtin[] = {"cd", "help", "exit"}; // declaring built-in before since help command shows them
int (*builtin_func[]) (char **) = {&sh_cd, &sh_help, &sh_exit}; // an array of pointers to the functions
int builtin_num = sizeof(builtin)/sizeof(char *); 

// change directory
int sh_cd(char **args){
    // check for errors
    if(args[1] == NULL){           // no argument to cd
        fprintf(stderr, "sh: expected argument to 'cd'\n");
    }else{                         // error in changing directory
        if(chdir(args[1]!=0)){     // changes direcotry or report error
            perror("sh");
        }
    }
    return 1;
}

// shell help
int sh_help(char **args){
    int i;
    printf("Shell Help\n");
    printf("You can execute programs by typing them and hitting enter.\nShell also has other built-in commands such as:\n");
    for(i=0;i<builtin_num;i++){
        printf(" %s\n", builtin[i]);
    }
    printf("Use man command for info on other programs.\n");
    return 1;

}

// exit command
int sh_exit(char **args){
    return 0;
}


// Execute commands

int sh_execute(char **args){
    int i;
    if(args[0] == NULL){             // empty command
        return 1;
    }
    for(i=0;i<builtin_num;i++){      // built-in commands
        if(strcmp(args[0], builtin[i])==0){
            return (*builtin_func[i])(args);
        }
    }
    return sh_launch(args);          // execute programs
}