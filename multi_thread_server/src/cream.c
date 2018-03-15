#include "cream.h"
#include "queue.h"
#include "utils.h"
#include "hashmap.h"
#include "string.h"
#include "csapp.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

queue_t *queue;
hashmap_t *hashmap;

typedef struct args_t {
    int num_workers;
    char* port_number;
    int max_entries;
    int help;
} args_t;

void queue_free_function(void *item) {
    free(item);
}

void free_map(map_key_t key, map_val_t val){
    free(key.key_base);
    free(val.val_base);
}

args_t *parse_args(int argc, char *argv[]){

    args_t *arg = Malloc(sizeof(args_t));
    int i=1;
    char help[] = "-h";
    while(i<argc){
        if(memcmp(help, argv[i],2) == 0){
            arg->help = 1;
            return arg;
        }
        i++;
    }
    arg->num_workers = atoi(argv[1]);
    arg->port_number = strdup(argv[2]);
    arg->max_entries = atoi(argv[3]);
    arg->help = 0;
    return arg;
}

void server_init(args_t *args){
    queue = create_queue();
    hashmap = create_map(args->max_entries, jenkins_one_at_a_time_hash, free_map);
}

int handle_put(int connfd, request_header_t request_header){
    char *key = Malloc(MAX_KEY_SIZE);
    char *val = Malloc(MAX_VALUE_SIZE);
    map_key_t map_key;
    map_val_t map_val;
    struct response_header_t response_header;
    if(request_header.key_size<MIN_KEY_SIZE || request_header.key_size>MAX_KEY_SIZE || \
        request_header.value_size<MIN_VALUE_SIZE || request_header.value_size>MAX_VALUE_SIZE){
            response_header.response_code = BAD_REQUEST;
            response_header.value_size = 0;
        }
        else{
            if(Rio_readn(connfd, key, request_header.key_size)<0){
                    return -1;
            }
            if(Rio_readn(connfd, val, request_header.value_size)<0){
                    return -1;
            }
            map_key.key_base = key;
            map_key.key_len = request_header.key_size;
            map_val.val_base = val;
            map_val.val_len = request_header.value_size;
            if(put(hashmap, map_key, map_val, true) == true){
                response_header.response_code = OK;
                response_header.value_size = 0;
            }
            else{
                response_header.response_code = BAD_REQUEST;
                response_header.value_size = 0;
            }
        }
        if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
            return -1;
        }
        return 0;
}

int handle_get(int connfd, request_header_t request_header){
    char *key = Malloc(MAX_KEY_SIZE);
    map_key_t map_key;
    struct response_header_t response_header;
    if(request_header.key_size<MIN_KEY_SIZE || request_header.key_size>MAX_KEY_SIZE){
        response_header.response_code = BAD_REQUEST;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
            return -1;
        }
    }
    else{
        if(Rio_readn(connfd, key, request_header.key_size)<0){
            return -1;
        }
        map_key.key_base = key;
        map_key.key_len = request_header.key_size;
        map_val_t map_val;
        map_val = get(hashmap, map_key);
        if(map_val.val_base == NULL){
            response_header.response_code = NOT_FOUND;
            response_header.value_size = 0;
            if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
                return -1;
            }
        }
        else{
            response_header.response_code = OK;
            response_header.value_size = map_val.val_len;
            if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
                return -1;
            }
            if(Rio_writen(connfd, map_val.val_base, map_val.val_len)<0){
                return -1;
            }
        }
    }
    return 0;
}

int handle_evict(int connfd, request_header_t request_header){
    char *key = Malloc(MAX_KEY_SIZE);
    map_key_t map_key;
    struct response_header_t response_header;
    if(request_header.key_size<MIN_KEY_SIZE || request_header.key_size>MAX_KEY_SIZE){
        response_header.response_code = BAD_REQUEST;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
            return -1;
        }
    }
    else{
        if(Rio_readn(connfd, key, request_header.key_size)<0){
            return -1;
        }
        map_key.key_base = key;
        map_key.key_len = request_header.key_size;
        delete(hashmap, map_key);
        response_header.response_code = OK;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
            return -1;
        }
    }
    return 0;
}

int handle_clear(int connfd){
    struct response_header_t response_header;
    if(clear_map(hashmap) == true){
        response_header.response_code = OK;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(struct response_header_t))<0){
            return -1;
        }
    }
    else{
        response_header.response_code = BAD_REQUEST;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(struct response_header_t))<0){
            return -1;
        }

    }
    return 0;
}

int handle_request(int connfd){
    rio_t rio;
    struct request_header_t request_header;
    Rio_readinitb(&rio, connfd);
    if(Rio_readn(connfd, &request_header, sizeof(request_header))<0){
        return -1;
    }

    if(request_header.request_code == PUT){
        handle_put(connfd, request_header);
    }
    else if(request_header.request_code == GET){
        handle_get(connfd, request_header);
    }
    else if(request_header.request_code == EVICT){
        handle_evict(connfd, request_header);
    }
    else if(request_header.request_code == CLEAR){
        handle_clear(connfd);
    }
    else{
        struct response_header_t response_header;
        response_header.response_code = UNSUPPORTED;
        response_header.value_size = 0;
        if(Rio_writen(connfd, &response_header, sizeof(response_header))<0){
            return -1;
        }
    }
    return 0;
}



void *thread(void *vargp){
    Pthread_detach(pthread_self());
    while(1){
        int *connfd;
        if((connfd = dequeue(queue))==NULL)
            app_error("[ERROR]: dequeue error.\n");
        else{
            handle_request(*connfd);
            Close(*connfd);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc > 5 || argc < 2)
        app_error("invalid number of arguments.\n");

    args_t *args = parse_args(argc, argv);
    if(args->help == 1){
        printf("./cream [-h] NUM_WORKERS PORT_NUMBER MAX_ENTRIES\n\
        -h                 Displays this help menu and returns EXIT_SUCCESS.\n\
        NUM_WORKERS        The number of worker threads used to service requests.\n\
        PORT_NUMBER        Port number to listen on for incoming connections.\n\
        MAX_ENTRIES        The maximum number of entries that can be stored in `cream`'s underlying data store.\n");
        exit(EXIT_SUCCESS);
    }

    signal(SIGPIPE, SIG_IGN);

    server_init(args);

    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    listenfd = Open_listenfd(args->port_number);
    pthread_t tid;
    for(int i=0; i<args->num_workers; i++){
        Pthread_create(&tid, NULL, thread, NULL);

    }
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        int *tmp = Malloc(sizeof(int));
        memcpy(tmp, &connfd, sizeof(int));
        if(enqueue(queue, tmp) == false)
            app_error("[ERROR]: enqueue error.\n");
    }

    exit(EXIT_SUCCESS);
}
