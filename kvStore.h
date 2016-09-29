
//A set size of the hashTable. using 100 for testing purposes for now.
#define TABLE_SIZE 100

//dataNode structure.
typedef struct node{
        int hash; //the key hash
        char* key; //the raw key
        char* value; //the value data
        int vSize; //size of data
        struct node* next; // next node in linked list.
}dataNode;

//Struct of hashTable
typedef struct kvHashtable {
        int size;
        dataNode** table;
}hashTable;

//Global variable for the hashTable. Memory allocated when storage is initialized.
hashTable* kvTable;

//kvStore.c function definitions.
int hashKey(char* key);
int addValue(char* key, char* value, int vSize);
int checkKey(char* key);
char* getValue(char* key, int* vSize);
void clearDB();
