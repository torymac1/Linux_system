#ifndef SFISH_H
#define SFISH_H

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

#endif



// for(int i=0; i< com_argc; i++){
    //     if (strcmp(com_argv[i], "<")==0){
    //         strcpy(input_file, com_argv[i+1]);
    //         //memset(com_argv[i], 0, sizeof(com_argv[i]));
    //         free(com_argv[i]);
    //         res = 101; //com [argv] < input_file
    //     }
    //     if(strcmp(com_argv[i], ">")==0){
    //         strcpy(output_file, com_argv[i+1]);
    //         //memset(com_argv[i], 0, sizeof(com_argv[i]));
    //         free(com_argv[i]);
    //         res = 102; //com [argv] > output_file
    //     }
    // }