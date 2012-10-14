
/*
 * Helper functions for treating node Buffer instances as C "pointers".
 */

#include "v8.h"
#include "node_buffer.h"

/*
 * Called when the "pointer" is garbage collected.
 */

inline static void wrap_pointer_cb(char *data, void *hint) {
  //fprintf(stderr, "wrap_pointer_cb\n");
}

/*
 * Wraps "ptr" into a new SlowBuffer instance with size "length".
 */

inline static v8::Handle<v8::Value> WrapPointer(char *ptr, size_t length) {
  void *user_data = NULL;
  node::Buffer *buf = node::Buffer::New(ptr, length, wrap_pointer_cb, user_data);
  return buf->handle_;
}

/*
 * Wraps "ptr" into a new SlowBuffer instance with length 0.
 */

inline static v8::Handle<v8::Value> WrapPointer(char *ptr) {
  return WrapPointer(ptr, 0);
}

/*
 * Unwraps Buffer instance "buffer" to a C `char *` with the offset specified.
 */

inline static char * UnwrapPointer(v8::Handle<v8::Value> buffer, int64_t offset) {
  return node::Buffer::Data(buffer.As<Object>()) + offset;
}

/*
 * Unwraps Buffer instance "buffer" to a C `char *` (no offset applied).
 */


inline static char * UnwrapPointer(v8::Handle<v8::Value>) {
  return node::Buffer::Data(buffer.As<Object>);
}
