#include <bits/stdc++.h>
using namespace std;

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE 2
#define ARROW_FLAG_MAP_KEYS_SORTED 4

extern "C" {

// The copy-pasted structs from the docs
struct ArrowSchema {
  // Array type description
  const char* format;
  const char* name;
  const char* metadata;
  int64_t flags;
  int64_t n_children;
  struct ArrowSchema** children;
  struct ArrowSchema* dictionary;

  // Release callback
  void (*release)(struct ArrowSchema*);
  // Opaque producer-specific data
  void* private_data;
};

struct ArrowArray {
  // Array data description
  int64_t length;
  int64_t null_count;
  int64_t offset;
  int64_t n_buffers;
  int64_t n_children;
  const void** buffers;
  struct ArrowArray** children;
  struct ArrowArray* dictionary;

  // Release callback
  void (*release)(struct ArrowArray*);
  // Opaque producer-specific data
  void* private_data;
};

// Release functions to be called by consumers once the work is done. Don't mind for now
static void release_int64_type(struct ArrowSchema* schema) {
   // Mark released
   schema->release = NULL;
}

// The Schema of the int32_t "Arrow Array"
void export_int64_type(struct ArrowSchema* schema) {
   *schema = (struct ArrowSchema) {
      // Type description
      .format = "l",
      .name = "",
      .metadata = NULL,
      .flags = 0,
      .n_children = 0,
      .children = NULL,
      .dictionary = NULL,
      // Bookkeeping
      .release = &release_int64_type
   };
}

// Release functions to be called by consumers once the work is done. Don't mind for now
static void release_int64_array(struct ArrowArray* array) {
   assert(array->n_buffers == 2);
   // Free the buffers and the buffers array
   free((void *) array->buffers[1]);
   free(array->buffers);
   // Mark released
   array->release = NULL;
}

// The actual definition of the int32_t Arrow Array
void export_int64_array(const int32_t* data, int64_t nitems,
                        struct ArrowArray* array) {
   // Initialize primitive fields
   *array = (struct ArrowArray) {
      // Data description
      .length = nitems,
      .null_count = 0,
      .offset = 0,
      .n_buffers = 2,
      .n_children = 0,
      .buffers = NULL,
      .children = NULL,
      .dictionary = NULL,
      // Bookkeeping
      .release = &release_int64_array
   };
   // Allocate list of buffers
   array->buffers = (const void**) malloc(sizeof(void*) * array->n_buffers);
   assert(array->buffers != NULL);
   array->buffers[0] = NULL;  // no nulls, null bitmap can be omitted
   array->buffers[1] = data;
}

}