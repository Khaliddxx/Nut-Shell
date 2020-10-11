#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <libgen.h>
#include <time.h>


#define BUFFER_LEN 1024
#define clear() printf("\033[H\033[J")

size_t read_command(char *cmd) 
{
    if (!fgets(cmd,BUFFER_LEN,stdin))           // get command and put it in line
        exit(0);
                                               // if user hits CTRL+D break
    size_t length = strlen(cmd);                // get command length
    if(cmd[length-1] == '\n')
    cmd[length-1] ='\0';                        // clear new line
    return strlen(cmd);                         // return length of the command read
}


int build_args(char * cmd, char ** argv) 
{
    char *token;                                //split command into separate strings
    token = strtok(cmd," ");
    int i = 0;
    while (token!=NULL)
    {                                           // loop for all tokens
        argv[i]=token;                          // store token
    token = strtok(NULL," ");                   // get next token
    i++;                                        // increment number of tokens
    }

    argv[i]=NULL;                               //set last value to NULL for execvp
    return i;                                   // return number of tokens
}


// function for finding and parsing pipes
int build_piped_args(char* cmd, char** argvp)
{
    int x;
    for (x=0; x<2; x++)
    {
        argvp[x] = strsep(&cmd, "|");
        if (argvp[x] == NULL)
            break;
    }

    if (argvp[1] == NULL)
        return 0;
    else
        return 1;
    
}

// Functions for built in commands
int handleCustomCmds(char** cmdParsed)
{
    int noOfCmds = 3, i, switchArg = 0;
    char* listOfCmds[noOfCmds];

    listOfCmds[0] = "exit";
    listOfCmds[1] = "cd";
    // To add other commands

    for (i=0; i<noOfCmds; i++)
    {
        if (strcmp(cmdParsed[0], listOfCmds[i]) == 0)
            switchArg = i+1;
            break;
    }

    switch (switchArg)
    {
        case 1:
            exit(0);
        case 2:
            chdir(cmdParsed[1]);
        default:
            break;
        
    }
}


void execc (char** cmdParsed)
{

    // Fork a child
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("\nFailed forking child!");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(cmdParsed[0], cmdParsed) < 0)
        {
            printf("\nCouldn't execvp command!");
            exit(0);
        }
        
    }
    else
    {
        // Wait for child to terminate
        wait(NULL);
        return;
    }
}

void execcPipedCmd(char** cmdParsed, char** parsedPipe)
{
    // 0 for read, 1 for write
    int pipee[2];
    pid_t p1, p2;

    if (pipe(pipee) < 0)
    {
        printf("\nPipe couldn't be init (pipe forgot to pipe)");
        return;
    }

    p1 = fork();
    if (p1 < 0)
    {
        printf("\nCouldn't fork p1");
        return;
    }

    if (p1 == 0)
    {
        // Child 1 execc, only writes to p2
        close(pipee[0]);
        dup2(pipee[1], STDOUT_FILENO);
        close(pipee[1]);

        if (execvp(cmdParsed[0], cmdParsed) < 0)
        {
            printf("\nCouldn't execvp command 1 from pipe");
            exit(0);
        }

        else
        {
            // Execute parent
            p2 = fork();

            if (p2 < 0)
            {
                printf("\nCouldn't fork p2");
                return;
            }
        // execvp child 2, only reads from p1
        if (p2 == 0)
        {
            close(pipee[1]);
            dup2(pipee[0], STDIN_FILENO);
            close(pipee[0]);

            if (execvp(parsedPipe[0], parsedPipe) < 0)
            {
                printf("\nCouldn't execvp the second command");
                exit(0);
            }

            else
            {
                // Waiting for the 2 children while parent executing
                wait(NULL);
                wait(NULL);
            }
        }

        }
    }
}



int processLine(char* cmd, char** parsed, char** parsedPipe)
{
    char* strpiped[2];
    int piped = 0;

    piped = build_piped_args(cmd, strpiped);

    if (piped)
    {
        build_args(strpiped[0], parsed);
        build_args(strpiped[1], parsedPipe);
    }

    else
        build_args(cmd, parsed);

    if (handleCustomCmds(parsed))
        return 1;
    else
        return 1 + piped;
}
    


void set_program_path (char * path, char * bin, char * prog) 
{
    memset(path,0,1024);                        // intialize buffer
    strcpy(path,bin);                           // copy /bin/ to file path
    strcat(path,prog);                          // add program to path
    for(int i=0; i<strlen(path); i++)           // delete newline
    if(path[i]=='\n')
        path[i]='\0';
}


void init_shell()
{
    clear();
    printf("\n\n\n ************************************");
    printf("\n\n\n\t\t NUT-SHELL \t\t");
    printf("\n\n\t       PUN INTENDED");
    printf("\n\n\t     USE AT YOUR RISK :d");
    printf("\n\n\n ************************************");

    char* user_name = getenv("USER");
    printf("\n\n\t USER IS: @%s \n\n\n", user_name);
    printf("\n");
    sleep(3);
    clear();

}

void printDir()
{
    char currentDir[1024];
    getcwd(currentDir, sizeof(currentDir));
    printf("\n%s >> ", currentDir);
}


int main()
{
    char line[BUFFER_LEN];                  // get command line
    char* argv[100];                        // user command
    char* bin = "/bin/";                    // set path at bin
    char path[1024];                        // full file path
    int argc;                               // arg count
    int execState;
    char** parseArgsPipedd;

    init_shell();
    
    while(1)
    {
        printDir();              // print shell prompt

// Read
        if(read_command(line) ==0) 
        {
                printf("\n");
                break;
        }                                   // CRTL+D pressed

        if(strcmp(line,"exit") ==0)
            exit(0);                        // exit


// Parse Commands
        argc = build_args(line,argv);       // build program argument
        //set_program_path(path,bin,argv[0]); // set program full path
        //int pid = fork();                   // fork child

        execState = processLine(line, argv, parseArgsPipedd);

// Execute
    //     if(pid==0){          
    //                                         // Child
    //         chdir(path);
            
    //         execvp(path,argv);              // if failed process is not replaced
    //                                         // then print error message
    //         fprintf(stderr,"Child process could not do execve\n");
    //         }
    //     else 
    //         wait(NULL);                     // Parent
    // }
    
    if (execState == 1)
        execc(argv);
    if (execState == 2)
        execcPipedCmd(argv, parseArgsPipedd);    
    }
    return 0;
}
