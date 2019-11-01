#include <iostream>
#include "My_Headers\index_list_t.h"
#define VERIFY_CONTEXT __FILE__, __PRETTY_FUNCTION__, __LINE__

#define HASH_TABLE_ASSERT(condition, message) \
if (!(condition)) {\
    htable_dump(table, ERR_STATE, message, filename, function, line);\
    assert(condition);\
    return false;\
}

char HASH_TABLE_LOG_NAME[] = "HTable_log.txt";
const size_t HASHT_DUMP_MSG_LENGTH = 100;
const char PARENT_CALL_STATE[] = "Parent call";

struct Hash_Table_t {
    size_t size;
    List_t* index;
    unsigned int (*hash)(char*, size_t);
};

void htable_init(Hash_Table_t* table, size_t  size, unsigned int (*hash)(char*, size_t));

void htable_init(Hash_Table_t* table, size_t size);

void htable_destruct(Hash_Table_t* table);

unsigned int htable_embedded_hash(char* buffer, size_t length);

int htable_push(Hash_Table_t* table, element_t elem);

bool htable_verify(Hash_Table_t* table, const char filename[], const char function[], int line);

void htable_dump(Hash_Table_t* table, const char state[], const char message[], const char filename[], const char function[], int line);

int main() {
    return 0;
}

void htable_init(Hash_Table_t *table, size_t size, unsigned int (*hash_function)(char *, size_t)) {
    table->size = size;
    table->hash = hash_function;
    table->index = (List_t*)calloc(size, sizeof(List_t));
    for (int i = 0; i < size; ++i) {
        list_init(&table->index[i], (size_t)0);
    }
}

void htable_init(Hash_Table_t *table, size_t size) {
    htable_init(table, size, htable_embedded_hash);
}


void htable_destruct(Hash_Table_t *table) {
    for (int i = 0; i < table->size; ++i) {
        list_destruct(&table->index[i]);
    }
    table->size = 0;
    table->hash = nullptr;
}


unsigned int htable_embedded_hash(char* buffer, size_t length) {
    assert(buffer);

    const unsigned int magic_constant = 0x5bd1e995;
    const unsigned int seed = 42;
    unsigned int hash = seed ^ length;
    /*operations with 4 byte blocks*/
    while (length >= 4)
    {
        /*merging together 4 bytes*/
        unsigned int word = buffer[0];//[-][-][-][0]
        word |= buffer[1] << 8;//[-][-][1][0]
        word |= buffer[2] << 16;//[-][2][1][0]
        word |= buffer[3] << 24;//[3][2][1][0]

        word *= magic_constant;
        unsigned int w = word;
        unsigned int s = word;
        word ^= word >> 24;
        word *= magic_constant;
        hash *= magic_constant;
        hash ^= word;
        buffer += 4;
        length -= 4;
    }
    /*operations with unused bytes*/
    switch (length) {
        case 3:
            hash ^= buffer[2] << 16;
        case 2:
            hash ^= buffer[1] << 8;
        case 1:
            hash ^= buffer[0];
            hash *= magic_constant;
    }

    hash ^= hash >> 13;
    hash *= magic_constant;
    hash ^= hash >> 15;

    return hash;
}

bool htable_verify(Hash_Table_t *table, const char *filename, const char *function, int line) {
#ifndef NDEBUG
    if (table == nullptr) {
        FILE* log = fopen(HASH_TABLE_LOG_NAME, "ab");
        fprintf(log, "ERROR. NULL pointer to Hash_Table %s; %s (%d).\n", filename, function, line);
        assert(table);
        return false;
    }
    HASH_TABLE_ASSERT(table->hash != nullptr, "Hash function not specified.")
    HASH_TABLE_ASSERT(table->size > 0, "Incorrect table size")

    for (int i = 0; i < table->size; ++i) {
        bool list_ok = list_verify(&table->index[i], filename, function, line);
        char msg[HASHT_DUMP_MSG_LENGTH] = {};
        sprintf(msg, "List in index[%d] error", i);
        if (!list_ok) {
            htable_dump(table, ERR_STATE, msg, filename, function, line);
            return false;
        }
    }

#ifdef OK_DUMP
    htable_dump(table, OK_STATE, "It's ok ^..^", filename, function, line);
#endif
#endif
    return true;
}

void htable_dump(Hash_Table_t* table, const char *state, const char *message, const char *filename, const char *function, int line) {
    FILE* log = fopen(HASH_TABLE_LOG_NAME, "ab");

    time_t now = time(nullptr);
    fprintf(log, "\nlog from %s"
                 "Hash_Table Dump(%s) from %s; %s (%d)\n"
                 "Hash_Table_t [%p] (%s)\n"
                 "{\n"
                 "\tsize = %d\n"
                 "\thash = %p\n"
                 "\tindex = %p\n"
                 "\t{<index_dump>\n", ctime(&now), message, filename, function, line, table, state, table->size, table->hash, table->index);
    for (int i = 0; i < table->size; ++i) {
        char msg[HASHT_DUMP_MSG_LENGTH] = {};
        sprintf(msg, "index[%d]", i);
        list_dump(&table->index[i], PARENT_CALL_STATE, msg, filename, function, line);
    }
    fclose(log);
}

