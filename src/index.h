#ifndef INDEX_H
#define INDEX_H

// default size for a new array
#define JUDY_MAX_ARRAY 1024

// maximum key length
#define MAX_KEY_SIZE 1024

// an index configuration struct
typedef struct indexConfig {
  int unique;
  void *idx;
} indexConfig;

// index data, count is always 1 if the index is unique
typedef struct indexData {
  int            count;
  int            size;
  void         **data;
} indexData;


indexConfig *createIndex (int);
int addToIndex (indexConfig *, const unsigned char *, void *);
int removeFromIndex (indexConfig *, const unsigned char *, void *);
indexData *getFromIndex (indexConfig *, const unsigned char *);
void vacuumIndex (indexConfig *);
void destroyIndex (indexConfig *);

#endif /* INDEX_H */
