
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

#include "sfish.h"
#include "debug.h"

#define PWD_SIZE 1024

char pwd[PWD_SIZE];

char dir[PWD_SIZE];

char *com_argv[100];
char *com_argv_pipe_0[100];
char *com_argv_pipe_1[100];
char *com_argv_pipe_2[100];
char *com_argv_pipe_3[100];
char *com_argv_pipe_4[100];
char *com_argv_pipe_5[100];
char *com_argv_pipe_6[100];
char *com_argv_pipe_7[100];
char *com_argv_pipe_8[100];
char *com_argv_pipe_9[100];

int pipe_time = 0;

char *com_argv_dp[100];
int com_argc;

char *input_file, *output_file, *pipe_file[100];


typedef struct {
  char *name;           /* User printable name of the function. */
  int (*func)();       /* Function to call to do the job. */
  char *doc;            /* Documentation for this function.  */
} COMMAND;


char *stripwhite (char *string){
    if(string == NULL)
        return string;
  register char *s, *t;

  for (s = string; whitespace (*s); s++);

  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

void update_pwd() {
    memset(pwd, 0, PWD_SIZE);
    getcwd(pwd, PWD_SIZE);
}

int com_pwd (ignore)
     char *ignore;
{
  char dir[PWD_SIZE], *s;

  s = getcwd (dir, PWD_SIZE);
  if (s == 0)
    {
      printf (BUILTIN_ERROR,"Error getting pwd");
      return 1;
    }

  printf ("%s\n", dir);
  return 0;
}


int com_cd (char *arg){


    char *s=stripwhite(arg);
    char path[PWD_SIZE];
    memset(path, 0, PWD_SIZE);
    strcpy(path, s);
    if(path[0]=='\0'){
        update_pwd();
        strncpy(path, getenv("HOME"), strlen(getenv("HOME")));
    }
    else if(strcmp(path,"-")==0){
        strncpy(path, pwd, strlen(pwd));
    }
    else{
        update_pwd();
        strcpy(path, s);
    }
    if (chdir (path) == -1){
        //perror (path);
        printf(BUILTIN_ERROR, "No such file or directory.");
        return 1;
    }
    //com_pwd ("");

    return (0);
}



int com_help (arg)
     char *arg;
{
  printf("See https://gitlab02.cs.stonybrook.edu/cse320/hw4-doc for details\n");
  return (0);
}

int com_exit (arg)
     char *arg;
{
  exit(EXIT_SUCCESS);
  return (0);
}


COMMAND commands[] = {
    { "cd", com_cd, "Change to directory DIR" },
    { "pwd", com_pwd, "Print the current working directory" },
    { "help", com_help, "See https://gitlab02.cs.stonybrook.edu/cse320/hw4-doc for details" },
    { "exit", com_exit, "exit" }
};


COMMAND *
find_command (name)
     char *name;
{
  register int i;

  for (i = 0; commands[i].name; i++)
    if (strcmp (name, commands[i].name) == 0)
      return (&commands[i]);

  return ((COMMAND *)NULL);
}


char **get_com_argv_pipe(int x){
    char **p;
    if(x==0) p = com_argv_pipe_0;
    if(x==1) p = com_argv_pipe_1;
    if(x==2) p = com_argv_pipe_2;
    if(x==3) p = com_argv_pipe_3;
    if(x==4) p = com_argv_pipe_4;
    if(x==5) p = com_argv_pipe_5;
    if(x==6) p = com_argv_pipe_6;
    if(x==7) p = com_argv_pipe_7;
    if(x==8) p = com_argv_pipe_8;
    if(x==9) p = com_argv_pipe_9;
    return p;
}

int parsing_arg(char *argv){
    memset(com_argv, 0, sizeof(com_argv));
    memset(com_argv_pipe_0, 0, sizeof(com_argv_pipe_0));
    memset(com_argv_pipe_1, 0, sizeof(com_argv_pipe_1));
    memset(com_argv_pipe_2, 0, sizeof(com_argv_pipe_2));
    memset(com_argv_pipe_3, 0, sizeof(com_argv_pipe_3));
    memset(com_argv_pipe_4, 0, sizeof(com_argv_pipe_4));
    memset(com_argv_pipe_5, 0, sizeof(com_argv_pipe_5));
    memset(com_argv_pipe_6, 0, sizeof(com_argv_pipe_6));
    memset(com_argv_pipe_7, 0, sizeof(com_argv_pipe_7));
    memset(com_argv_pipe_8, 0, sizeof(com_argv_pipe_8));
    memset(com_argv_pipe_9, 0, sizeof(com_argv_pipe_9));
    com_argc = 0;
    pipe_time = 0;
    char *arg;

    char *argv_copy=calloc(strlen(argv)+1, sizeof(char));
    strcpy(argv_copy, argv);
    //printf("%s\n", argv_copy);
    int res = 0;
    char *arg_out;
    if (strchr(argv_copy, '|')!=NULL){
        while((arg_out = strsep(&argv, "|"))!=NULL){
            char **com_argv_cur = get_com_argv_pipe(pipe_time);
            //memset(com_argv_cur, 0, sizeof(com_argv_cur));
            com_argc = 0;
            while((arg = strsep(&arg_out, " "))!=NULL){
                if(strlen(arg)!=0){
                    com_argv_cur[com_argc] = calloc(strlen(arg) + 1, sizeof(char));
                    strcpy(com_argv_cur[com_argc], arg);
                    com_argc++;
                }
            }
            pipe_time++;
        }
        res = 105;
        return res;
    }
    while((arg = strsep(&argv, " "))!=NULL){
        if(strchr(arg, '|')!=NULL || strchr(arg, '<')!=NULL || strchr(arg, '>')!=NULL)
            break;
        if(strlen(arg)!=0){
            com_argv[com_argc] = calloc(strlen(arg) + 1, sizeof(char));
            strcpy(com_argv[com_argc], arg);
            com_argc++;
        }
    }
    //only >, out
    if(strchr(argv_copy, '<')==NULL && strchr(argv_copy, '>')!=NULL && strchr(argv_copy, '|')==NULL){
        char *cur = strchr(argv_copy, '>');
        output_file = calloc(strlen(cur+1)+1, sizeof(char));
        strcpy(output_file, stripwhite(cur+1));
        res = 101;

    }
    //only <, in
    if(strchr(argv_copy, '<')!=NULL && strchr(argv_copy, '>')==NULL && strchr(argv_copy, '|')==NULL){
        char *cur = strchr(argv_copy, '<');
        input_file = calloc(strlen(cur+1)+1, sizeof(char));
        strcpy(input_file, stripwhite(cur+1));
        res = 102;
    }

    //both < and >
    if(strchr(argv_copy, '<')!=NULL && strchr(argv_copy, '>')!=NULL && strchr(argv_copy, '|')==NULL){
        // "<" before ">"
        if(strchr(argv_copy, '<') < strchr(argv_copy, '>')){
            char* cur = strchr(argv_copy, '<');
            char* cur_next = strchr(argv_copy, '>');
            int mid_len = (int)(cur_next - cur) - 1;
            input_file = calloc(mid_len, sizeof(char));
            strncpy(input_file, cur+1, mid_len);
            input_file = stripwhite(input_file);

            output_file = calloc(strlen(cur_next+1)+1, sizeof(char));
            strcpy(output_file, stripwhite(cur_next+1));
            res = 103;
        }
        // ">" before "<"
        if(strchr(argv_copy, '<') > strchr(argv_copy, '>')){
            char* cur = strchr(argv_copy, '>');
            char* cur_next = strchr(argv_copy, '<');
            int mid_len = (int)(cur_next - cur) - 1;
            output_file = calloc(mid_len, sizeof(char));
            strncpy(output_file, cur+1, mid_len);
            output_file = stripwhite(output_file);

            input_file = calloc(strlen(cur_next+1)+1, sizeof(char));
            strcpy(input_file, stripwhite(cur_next+1));
            res = 104;
        }
    }
    // for (int i=0; i<com_argc; i++)
    //     printf("%s\n",com_argv[i]);
    return res;
}

int pipe_proc(int in, int out, char *argv[]){
    pid_t pid=0;

    if((pid=fork())==0){

        if(in != 0){
            dup2(in, 0);
            close(in);
        }
        if(out != 1){
            dup2(out, 1);
            close(out);
        }
        int res;

        if((res = execvp(argv[0], argv))<0){
            printf(EXEC_ERROR, argv[0]);
            return res;
        }
    }
    return pid;

}

int
execute_line (char *line){
    register int i;
    COMMAND *command;
    char *word;
    char argv[100];
    memset(argv, 0, 100);
    strcpy(argv, line);   //argv for not builtin

    /* Isolate the command word. */
    i = 0;
    while (line[i] && whitespace (line[i]))
    i++;
    word = line + i;

    while (line[i] && !whitespace (line[i]))
    i++;

    if (line[i])
    line[i++] = '\0';
    command = find_command (word);

     /* Get argument to command, if any. */
    while (whitespace (line[i]))
        i++;

    word = line + i;

    //builtin command
    if (command){
        return ((*(command->func)) (word));
    }

    int dir_pipe = parsing_arg(argv);
    int pid;
    int status;
    int in, out;

    if(dir_pipe == 105){  //pipe
        int fd[2], in, i;
        in = 0; //stdin
        for(i=0; i< pipe_time; ++i){
            pipe(fd);
            //printf("%d %d\n",fd[0], fd[1]);
            if(i==pipe_time - 1){
                if((pid=fork())==0){
                    if(in!=0)
                        dup2(in, 0);
                    if(execvp(get_com_argv_pipe(i)[0], get_com_argv_pipe(i))<0){
                        printf(EXEC_ERROR, get_com_argv_pipe(i)[0]);
                        break;
                    }
                }
            }
            else{
                if(pipe_proc(in, fd[1], get_com_argv_pipe(i))<0){
                    printf(EXEC_ERROR, get_com_argv_pipe(i)[0]);
                    break;
                }
                else{
                    close(fd[1]);
                    in = fd[0];
                }
            }
        }
        close(fd[0]);
        close(fd[1]);
        while(waitpid(-1, &status, 0)>0);
        return 0;
    }
    if((pid=fork())==0){ //not pipe
        if(dir_pipe == 101){    //out
            out = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        else if(dir_pipe == 102){
            in = open(input_file, O_RDONLY);
            dup2(in, STDIN_FILENO);
            close(in);
        }
        else if(dir_pipe == 103 || dir_pipe == 104){
            int out = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(out, STDOUT_FILENO);
            close(out);
            int in = open(input_file, O_RDONLY);
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if(execvp(com_argv[0], com_argv)>0){
            exit(EXIT_SUCCESS);
        }
        else{
            printf(EXEC_NOT_FOUND, line);
            exit(EXIT_FAILURE);
        }
    }
    else{
        waitpid(pid, &status, 0);
        return 0;
        // do {
        //     wpid = waitpid(pid, &status, WUNTRACED);
        // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}

int main(int argc, char *argv[], char* envp[]) {
    char *input, *cmd;
    bool exited = false;


    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }
    update_pwd();

    do {
        //Modify the prompt
        char *home = getenv("HOME");
        int home_len = strlen(home);
        memset(dir, 0, PWD_SIZE);
        getcwd (dir, PWD_SIZE);
        if (strncmp(dir, home, home_len) == 0) {
            char dir_temp[PWD_SIZE];
            strcpy(dir_temp, dir + home_len);
            dir[0] = '~';
            strcpy(dir + 1, dir_temp);
        }

        //fflush(stdout);
        printf("%s :: sanwang",dir);
        //fflush(stdin);
        //get input
        input = readline(">>");

        write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));

        // If EOF is read (aka ^D) readline returns NULL
        cmd=stripwhite(input);
        if(cmd == NULL || strcmp(input, "\0")==0) {
            continue;
        }
        else{
            execute_line(cmd);
            add_history(cmd);
        }

    } while(!exited);

    // Readline mallocs the space for input. You must free it.
    rl_free(input);

    debug("%s", "user entered 'exit'");

    return 0;
}
