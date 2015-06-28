#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"
#include "judy64nb.h"

// find the index of the value entry, returning -1 if not found
static int indexOf (indexData *idx, void *value) {
  int i;

  for (i = 0; i < idx->count; i++) {
    if (idx->data[i] == value) {
      return i;
    }
  }

  return -1;
}

// remove an entry from the array
// all this does is move any index entries to fill an empty slot
static void removeFromArray (indexData *idx, int index) {
  int i;

  // start at the slot being removed
  for (i = index; i < idx->count - 1; i++) {
    // move the next entry into this slot
    idx->data[i] = idx->data[i + 1];
  }
}

// clean up an entry, freeing up whatever is prudent, and reallocating where needed
// this is part of the vacuum process
static void cleanEntry (Judy *idx, JudySlot *cell) {
  indexData *data;

  if (cell) {
    data = (indexData *) *cell;

    // if the cell is empty, delete the cell
    if (data->count == 0) {
      free(data->data);

      judy_del(idx);
      free(data);
    } else if (data->count < data->size) {
      // reallocate to shrink the entry
      data->data = (void *) realloc(data->data, sizeof(void *) * data->count);
      data->size = data->count;
    }
  }
}

// free up the cell data
static void freeEntry (indexData *data) {
  free(data->data);
  free(data);
}

// create an index - this needs to be called before anything else to set up the index
indexConfig *createIndex (int unique) {
  indexConfig *cnf;

  // allocate the base index configuration
  cnf = (indexConfig *) malloc(sizeof(indexConfig));

  // define whether the index is unique
  if (unique) {
    cnf->unique = 1;
  }

  // create the judy array
  cnf->idx = (void *) judy_open(JUDY_MAX_ARRAY, 0);

  return cnf;
}

// add a key/value pair to the index
int addToIndex (indexConfig *cnf, const unsigned char *key, void *value) {
  indexData *data;
  JudySlot *cell;

  // find or allocate the cell from the judy array
  cell = judy_cell((Judy *) cnf->idx, (uchar *) key, (size_t) strlen((const char *) key));

  if (cell && *cell) {
    // the cell already has a data entry
    data = (indexData *) *cell;

    // if unique, there are some checks that need to happen
    if (cnf->unique) {
      // index already contains this key, return "false"
      if (data->count) {
        return 0;
      } else {
        // allocate the data, and set the value
        data->data = (void *) malloc(sizeof(void *));
        data->data[0] = value;
        data->size = 1;
      }
    } else if (indexOf(data, value) == -1) { // check the index to see if this value exists
      // value does not exist, add it to the entry
      data->count++;

      // only realloc if needed, use any existing memory already allocated
      if (data->size < data->count) {
        data->data = (void *) realloc(data->data, sizeof(void *) * data->count);

        // the size has grown, set it to the count
        data->size = data->count;
      }

      // set the value
      data->data[data->count - 1] = value;
    }

    // set the cell address to data
    *cell = (JudySlot) data;

    return 1;
  } else if (cell) {
    // cell not found, go ahead and set up a data entry
    data = (indexData *) malloc(sizeof(indexData));

    // set the count and size to 1
    data->count = 1;
    data->size = 1;

    // allocate the actual data
    data->data = (void *) malloc(sizeof(void *));
    data->data[0] = value;

    // set the cell address to data
    *cell = (JudySlot) data;

    return 1;
  }

  return 0;
}

// find an entry, and remove it from the index
int removeFromIndex (indexConfig *cnf, const unsigned char *key, void *value) {
  indexData *data;
  JudySlot *cell;
  int current;

  // find the cell
  cell = judy_slot((Judy *) cnf->idx, (uchar *) key, (size_t) strlen((const char *) key));

  // found the cell, it is valid
  if (cell) {
    data = (indexData *) *cell;

    // if it is already gone, return false
    if (data->count == 0) {
      return 0;
    }

    // find the value in this entry
    current = indexOf(data, value);

    // value wasn't found, return false
    if (current == -1) {
      return 0;
    }

    // remove the entry from the array
    removeFromArray(data, current);

    // decrement the counter, ignore the size until the next vacuum
    data->count = data->count - 1;

    return 1;
  }

  return 0;
}

// get an entry from the index, return it or null
indexData *getFromIndex (indexConfig *cnf, const unsigned char *key) {
  indexData *data;
  JudySlot *cell;
  int current;

  // search for the cell for the key
  cell = judy_slot((Judy *) cnf->idx, (uchar *) key, (size_t) strlen((const char *) key));

  // found the cell
  if (cell) {
    data = (indexData *) *cell;

    // if the count is 0, consider it not found
    if (data->count == 0) {
      return NULL;
    }

    // otherwise return the whole entry, all values match
    return data;
  }

  return NULL;
}

// vacuum - clean up any empty spots
void vacuumIndex (indexConfig *cnf) {
  indexData *data;
  JudySlot *cell;
  uchar key[MAX_KEY_SIZE];

  // set to the start of the judy array
  cell = judy_strt ((Judy *) cnf->idx, NULL, 0);

  // if there is an initial cell, free it up
  if (cell) {
    cleanEntry(cnf->idx, cell);

    // while there are still keys to iterate through
    while (judy_nxt(cnf->idx)) {
      // get the key name
      judy_key(cnf->idx, (uchar *) key, MAX_KEY_SIZE);

      // get the next cell by key name
      cell = judy_slot((Judy *) cnf->idx, (uchar *) key, (size_t) strlen((const char *) key));

      // free the entry up
      cleanEntry(cnf->idx, cell);
    }
  }
}

// free all of the memory from the index
void destroyIndex (indexConfig *cnf) {
  JudySlot *cell;
  uchar key[MAX_KEY_SIZE];
  indexData *data;

  // set to the start of the judy array
  cell = judy_strt ((Judy *) cnf->idx, NULL, 0);

  if (cell) {
    // free it up if there is a cell
    freeEntry((indexData *) *cell);

    // while there are still cells
    while (judy_nxt(cnf->idx)) {
      // find the key
      judy_key(cnf->idx, (uchar *) &key, MAX_KEY_SIZE);

      // find an cell
      data = getFromIndex(cnf, key);

      // free it up
      freeEntry(data);
    }
  }

  // close the judy array
  judy_close(cnf->idx);

  // free the config
  free(cnf);
}
