#include "debug.h"
#include "utf.h"
#include "wrappers.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int opterr;
int optopt;
int optind;
char *optarg;

state_t *program_state;

void
parse_args(int argc, char *argv[])
{
  int i;
  char option;
  //char *joined_argv;

  //joined_argv = join_string_array(argc, argv);
  //info("argc: %d argv: %s", argc, joined_argv);
  //free(joined_argv);   //delete line

  //program_state = Calloc(1, sizeof(state_t));
  if (argc <= 1){
    USAGE(argv[0]);
    free(program_state);
    exit(EXIT_FAILURE);
  }

  for (i = 0; optind < argc && i < argc; ++i) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+e:")) != -1) {
      switch (option) {
        case 'e': {
          info("Encoding Argument: %s", optarg);
          if ((program_state->encoding_to = determine_format(optarg)) == 0)
            goto errorcase;
          break;
        }
        case '?': {
          if (optopt != 'h'){
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
            goto errorcase;
          }
          else{
            USAGE(argv[0]);
            free(program_state);
            exit(EXIT_SUCCESS);
          }
          break;
        errorcase:{
          USAGE(argv[0]);
          free(program_state);
          exit(EXIT_FAILURE);
          break;
        }
        }
        default: {
          break;
        }
      }
    }
    elsif(argv[optind] != NULL)
    {
      if (program_state->in_file == NULL) {
        program_state->in_file = argv[optind];
      }
      elsif(program_state->out_file == NULL)
      {
        program_state->out_file = argv[optind];
      }
      else{
        USAGE(argv[0]);
        free(program_state);
        exit(EXIT_FAILURE);
      }
      optind++;
    }
  }
  //free(joined_argv);
  //free(program_state);
  debug("infile %s  outfile  %s", program_state->in_file, program_state->out_file);
}

format_t
determine_format(char *argument)
{

  if (strcmp(argument, STR_UTF16LE) == 0)
    return UTF16LE;
  if (strcmp(argument, STR_UTF16BE) == 0)
    return UTF16BE;
  if (strcmp(argument, STR_UTF8) == 0)
    return UTF8;
  return 0;
}

const char*
bom_to_string(format_t bom){
  switch(bom){
    case UTF8: return STR_UTF8;
    case UTF16BE: return STR_UTF16BE;
    case UTF16LE: return STR_UTF16LE;
  }
  return "UNKNOWN";
}

char*
join_string_array(int count, char *array[])
{
  char *ret;
  //char charArray[count];
  int i;
  int len = 0, cur_str_len;

  int str_len = array_size(count, array);
  //ret = charArray;
  ret = Malloc(sizeof(char)*str_len);

  for (i = 0; i < count; ++i) {
    cur_str_len = strlen(array[i]);
    memecpy(ret + len, array[i], cur_str_len);
    len += cur_str_len;
    memecpy(ret + len, " ", 1);
    len += 1;
  }

  return ret;
}

int
array_size(int count, char *array[])
{
  int i, sum = 1; /* NULL terminator */
  for (i = 0; i < count; ++i) {
    sum += strlen(array[i]);
    ++sum; /* For the spaces */
  }
  return sum+1;
}

void
print_state()
{
//errorcase:
  if (program_state == NULL) {
    error("program_state is %p", (void*)program_state);
    exit(EXIT_FAILURE);
  }
  info("program_state {\n"
         "  format_t encoding_to = 0x%X;\n"
         "  format_t encoding_from = 0x%X;\n"
         "  char *in_file = '%s';\n"
         "  char *out_file = '%s';\n"
         "};\n",
         program_state->encoding_to, program_state->encoding_from,
         program_state->in_file, program_state->out_file);
}
