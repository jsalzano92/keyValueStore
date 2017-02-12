#include <stdlib.h>
#include <string.h>
#include "kvStore.h"

//Allocates the memory for a new kvTable structure and the hashtable of pointers
int initDatabase() {
        kvTable = malloc(sizeof(hashTable));
        kvTable->size = TABLE_SIZE;
        kvTable->table = calloc(TABLE_SIZE, sizeof(dataNode*));
        return 1;
}

//Takes a key and a value from the front end of the server and stores it in the hashtable.
int addValue(char* key, char* value, int vSize) {
        //Takes the key and hashes it, then mods the hash to fit in the table.
        unsigned long hash = hashKey(key);
        int index = hash % TABLE_SIZE;
        //Checks if the key already exists. If it does then return as a failure.
        if(checkKey(key))
                return 0;
        //Creates a new dataNode and stores the information of the key-value.
        dataNode* newNode = malloc(sizeof(dataNode));
        newNode->hash = hash; //For faster searching
        newNode->key = key; //For preciese searching
        newNode->value = value; //The data
        newNode->vSize = vSize; //For creating a header sent to the client
        //If there is a collision make a linked list of dataNodes.
        if(kvTable->table[index])
                newNode->next = kvTable->table[index];
        else
                newNode->next = NULL;
        //Add the dataNode to the hashtable
        kvTable->table[index] = newNode;
        //Return success.
        return 1;
}

//Checks the key given to see if it exists in the hashtable.
int checkKey(char* key) {
        //Hashes the key
        unsigned long hash = hashKey(key);
        //Mods the hash to find the index to search in.
        int index = hash % TABLE_SIZE;
        
        //Start at the begining of the linked list.
        dataNode* current = kvTable->table[index];

        //If there isn't a linked list then we know the key doesn't exist.
        if(!current)
                return 0;

        //Iterate through the linked list.
        while(current) {
                //If the hashes match...
                if(current->hash == hash) {
                        //...Then check the key. If it matches return true that the key is found.
                        if(strcmp(current->key, key) == 0) {
                                return 1;
                        }
                }
                current = current->next;
        }
        //If the key is not found return false.
        return 0;
}

//Returns the data associated with the key given.
char* getValue(char* key, int* vSize) {
        //Hashes the key
        unsigned long hash = hashKey(key);
        //Mods the hash to find the index to search in.
        int index = hash % TABLE_SIZE; 

        //Start at the begining of the linked list.
        dataNode* current = kvTable->table[index];


        //If there isn't a linked list then we know the key doesn't exist.
        if(!current)
                return NULL;

        //Iterate through the linked list.
        while(current) {
                //If the hashes match...
                if(current->hash == hash) {
                        //...Then check the key. If the key matches then set the vSize to the size and returns the value data.
                        if(strcmp(current->key, key) == 0) {
                                *vSize = current->vSize;
                                return current->value;
                        }
                
                }
                current = current->next;
        }
        //If the key is not found return a NULL pointer.
        return NULL;
}

// Using DJB2 hashing algorithm found on stack overflow and multiple other online sources.
unsigned long hashKey(char* key) {
        unsigned long hash = 5381;
        int c;

        while (*key) {
                c = *key;
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
                key++;
        }

        return hash;
}

//Clears the database and all nodes. Frees all memory allocated in the preceiding functions.
void clearDB() {
        for(int i = 0; i < TABLE_SIZE; i++) {
                dataNode* current = kvTable->table[i];
                while(current) {
                        dataNode* next = current->next;
                        free(current->key);
                        free(current->value);
                        free(current);
                        current = next;
                }
        }
        free(kvTable->table);
        free(kvTable);
}
