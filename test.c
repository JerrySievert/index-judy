#include <stdio.h>
#include "src/index.h"

int main () {
  indexConfig *normal;
  indexConfig *unique;
  indexData *data;
  int ret;
  char *val1 = "test1";
  char *val2 = "test2";
  char *val3 = "test3";

  normal = createIndex(0);
  unique = createIndex(1);

  ret = addToIndex(normal, "key1", (void *) val1);
  printf("addtoIndex normal, key1 - %d\n", ret);

  ret = addToIndex(normal, "key2", (void *) val2);
  printf("addtoIndex normal, key2 - %d\n", ret);

  ret = addToIndex(normal, "key1", (void *) val3);
  printf("addtoIndex normal, key1 - %d\n", ret);

  data = getFromIndex(normal, "key1");
  printf("getting key1, count %d (%s, %s)\n", data->count, (char *)data->data[0], (char *)data->data[1]);

  data = getFromIndex(normal, "key2");
  printf("getting key2, count %d (%s)\n", data->count, (char *)data->data[0]);

  printf("removing key1 from index\n");
  removeFromIndex(normal, "key1", (void *) val1);

  data = getFromIndex(normal, "key1");
  printf("getting key1, count %d, size %d\n", data->count, data->size);

  printf("vacuuming\n");
  vacuumIndex(normal);

  data = getFromIndex(normal, "key1");
  printf("getting key1, count %d, size %d (%s)\n", data->count, data->size, (char *)data->data[0]);

  ret = addToIndex(unique, "key1", (void *) "test1");
  printf("addtoIndex unique, key1 - %d\n", ret);

  ret = addToIndex(unique, "key1", (void *) "test1");
  printf("addtoIndex unique, key1 - %d\n", ret);

  destroyIndex(unique);
  destroyIndex(normal);
  exit(1);
}
