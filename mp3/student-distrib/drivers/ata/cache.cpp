#include "ata_priv.h"

/*
 * This code is copied from https://github.com/mallardtheduck/osdev and slightly revised.
 */

namespace ata {

constexpr size_t cache_size = 4096;

struct cache_element {
    bool inuse;
    size_t deviceid;
    size_t sector;
    size_t usecount;
    char data[ATA_SECTOR_SIZE];
};

static cache_element cache[cache_size];

static bool find_free(size_t &index){
    for(size_t i=0; i<cache_size; ++i){
        if(!cache[i].inuse){
            index=i;
            return true;
        }
    }
    return false;
}

static void prune_cache(){
    size_t min_uses=0xFFFFFFFF;
    for(size_t i=0; i<cache_size; ++i){
        if(cache[i].inuse && cache[i].usecount < min_uses) min_uses=cache[i].usecount;
    }
    for(size_t i=0; i<cache_size; ++i){
        if(cache[i].inuse){
            cache[i].usecount-=min_uses;
            if(!cache[i].usecount) cache[i].inuse=false;
        }
    }
}

void cache_add(size_t deviceid, size_t sector, char *data){
    size_t index=0;
    if(!find_free(index)){
        prune_cache();
        if(!find_free(index)) return;
    }
    cache[index].deviceid=deviceid;
    cache[index].sector=sector;
    cache[index].usecount=0;
    memcpy(cache[index].data, data, ATA_SECTOR_SIZE);
    cache[index].inuse=true;
}

bool cache_get(size_t deviceid, size_t sector, char *data){
    for(size_t i=0; i<cache_size; ++i){
        if(cache[i].inuse && cache[i].deviceid==deviceid && cache[i].sector==sector){
            cache[i].usecount++;
            memcpy(data, cache[i].data, ATA_SECTOR_SIZE);
            return true;
        }
    }
    return false;
}

void cache_drop(size_t deviceid, size_t sector){
    for(size_t i=0; i<cache_size; ++i) {
        if (cache[i].inuse && cache[i].deviceid == deviceid && cache[i].sector == sector) {
            cache[i].inuse=false;
        }
    }
}

}   // namespace ata
