#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include "utils.h"
#include "string.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

// void free_map(map_key_t key, map_val_t val){
//     free(key.key_base);
//     free(val.val_base);
// }

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if (capacity <= 0){
        errno = EINVAL;
        return NULL;
    }
    hashmap_t *map;

    if((map = calloc(1, sizeof(hashmap_t))) == NULL)
        return NULL;
    map->capacity = capacity;
    map->size = 0;
    map->hash_function = hash_function;
    map->destroy_function = destroy_function;
    map->num_readers = 0;
    map->invalid = false;
    if(pthread_mutex_init(&map->write_lock, NULL)!=0)
        return NULL;
    if(pthread_mutex_init(&map->fields_lock, NULL)!=0)
        return NULL;
    if((map->nodes = calloc(capacity, sizeof(map_node_t))) == NULL)
        return NULL;
    return map;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    if(self == NULL || self->invalid == true || key.key_base == NULL || val.val_base == NULL){
        errno = EINVAL;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);
    if(self->capacity == self->size && force == false){
        errno = ENOMEM;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }

    //not full
    if(self->size < self->capacity){
        int index = get_index(self, key);
        map_node_t *tmp = self->nodes+index;
        //empty slot
        while(1){
            if(tmp->key.key_base == NULL || tmp->tombstone == true){
                tmp->key = key;
                tmp->val = val;
                tmp->tombstone = false;
                tmp->start = clock();
                break;
            }
            else if(key.key_len == tmp->key.key_len){
                if(memcmp(key.key_base, tmp->key.key_base, key.key_len) == 0){
                    tmp->val = val;
                    tmp->tombstone = false;
                    tmp->start = clock();
                    break;
                }
            }
            if(index == (int)self->capacity - 1){
                index = 0;
                tmp = self->nodes;
            }
            else{
                tmp++;
                index++;
            }
        }
        self->size++;

    }
    //full and force == true
    else{
        int index = get_index(self, key);
        map_node_t *tmp = self->nodes+index;
        self->destroy_function(tmp->key, tmp->val);
        tmp->key = key;
        tmp->val = val;
        tmp->tombstone = false;
    }
    pthread_mutex_unlock(&self->write_lock);
    return true;
}

int get_true_index(hashmap_t *self, map_key_t key){
    if(self == NULL || self->invalid == true || key.key_base == NULL){
        errno = EINVAL;
        return -1;
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1)
        pthread_mutex_lock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);

    int index = get_index(self, key);
    map_node_t *tmp = self->nodes+index;
    int i = 0;
    while(i < self->capacity){
        //tombstone == true
        if(tmp->tombstone == true){
            if(index == (int)self->capacity - 1){
                index = 0;
                tmp = self->nodes;
                i++;
            }
            else{
                i++;
                index++;
                tmp++;
            }
            continue;
        }
        //NULL and break
        if(tmp->key.key_base == NULL){
            index = -1;
            break;
        }
        //not NULL, check if key is equal
        else if(key.key_len == tmp->key.key_len){
            if(memcmp(key.key_base, tmp->key.key_base, key.key_len) == 0){
                break;
            }
        }
        if(index == (int)self->capacity - 1){
            index = 0;
            tmp = self->nodes;
            i++;
        }
        else{
            i++;
            index++;
            tmp++;
        }
    }
    //not find
    if(i == self->capacity){
        index = -1;
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0)
        pthread_mutex_unlock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
    return index;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    if(self == NULL || self->invalid == true || key.key_base == NULL){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1)
        pthread_mutex_lock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);

    int index = get_index(self, key);
    map_node_t *tmp = self->nodes+index;
    map_val_t rtn;
    int i = 0;
    while(i < self->capacity){
        //NULL and break
        if(tmp->key.key_base == NULL){
            rtn.val_base = NULL;
            rtn.val_len = 0;
            break;
        }
        //tombstone == true, continue
        if(tmp->tombstone == true || (clock() - tmp->start)/CLOCKS_PER_SEC>TTL){
            tmp->tombstone = true;
            if(index == (int)self->capacity - 1){
                index = 0;
                tmp = self->nodes;
                i++;
            }
            else{
                i++;
                index++;
                tmp++;
            }
            continue;
        }
        //not NULL, check if key is equal
        else if(key.key_len == tmp->key.key_len){
            if(memcmp(key.key_base, tmp->key.key_base, key.key_len) == 0){
                rtn = tmp->val;
                break;
            }
        }
        if(index == (int)self->capacity - 1){
            index = 0;
            tmp = self->nodes;
            i++;
        }
        else{
            i++;
            index++;
            tmp++;
        }
    }
    //not find
    if(i == self->capacity){
        rtn.val_base = NULL;
        rtn.val_len = 0;
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0)
        pthread_mutex_unlock(&self->write_lock);
    pthread_mutex_unlock(&self->fields_lock);
    return MAP_VAL(rtn.val_base, rtn.val_len);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    if(self == NULL || self->invalid == true ||  key.key_base == NULL || get(self, key).val_base == NULL){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    int index = get_true_index(self, key);
    if (index == -1){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    pthread_mutex_lock(&self->write_lock);
    map_node_t *rtn = self->nodes+index;
    bool t = rtn->tombstone;
    rtn -> tombstone = true;
    self->size--;
    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(rtn->key.key_base, rtn->key.key_len), MAP_VAL(rtn->val.val_base, rtn->val.val_len), t);
}

bool clear_map(hashmap_t *self) {
    if (self == NULL || self->invalid == true){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    map_node_t *tmp = self->nodes;
    for (int i=0; i<self->capacity; i++){
        if(tmp->key.key_base != NULL && tmp->val.val_base != NULL){
            tmp->tombstone = true;
            self->destroy_function(tmp->key, tmp->val);
            tmp->key.key_base = NULL;
            tmp->val.val_base = NULL;
            tmp->key.key_len = 0;
            tmp->val.val_len = 0;
        }
        tmp++;
    }
    self->size = 0;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}

bool invalidate_map(hashmap_t *self) {
    if (self == NULL || self->invalid == true){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    free(self->nodes);
    self->invalid = true;
    pthread_mutex_unlock(&self->write_lock);
    return false;
}
