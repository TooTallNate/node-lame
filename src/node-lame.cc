/*
 * Copyright (c) 2011, Nathan Rajlich <nathan@tootallnate.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <lame/lame.h>

using namespace v8;
using namespace node;

namespace {

/* Wrapper ObjectTemplate to hold `lame_t` instances */
Persistent<ObjectTemplate> gfpClass;


/* get_lame_version() */
Handle<Value> node_get_lame_version (const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New(get_lame_version()));
}

/* lame_close() */
Handle<Value> node_lame_close (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);
  lame_close(gfp);
  return Undefined();
}


/* malloc()'s a `lame_t` struct and returns it to JS land */
Handle<Value> node_malloc_gfp (const Arguments& args) {
  HandleScope scope;

  lame_global_flags *gfp = lame_init();

  // disable id3v2 in the encode stream;
  // user must call lame_getid3v2_tag() manually
  lame_set_write_id3tag_automatic(gfp, 0);

  Local<Object> wrapper = gfpClass->NewInstance();
  wrapper->SetPointerInInternalField(0, gfp);

  return scope.Close(wrapper);
}


/* lame_encode_buffer_interleaved() */
Handle<Value> node_lame_encode_buffer_interleaved (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);
  // Turn into 'short int []'
  char *input = Buffer::Data(args[1]->ToObject());
  short int *pcm = (short int *)input;
  // num in 1 channel, not entire pcm array
  int num_samples = args[2]->Int32Value();
  Local<Object> outbuf = args[3]->ToObject();
  unsigned char *mp3buf = (unsigned char *)Buffer::Data(outbuf);
  int mp3buf_size = Buffer::Length(outbuf);

  int b = lame_encode_buffer_interleaved(gfp, pcm, num_samples, mp3buf, mp3buf_size);
  return scope.Close(Integer::New(b));
}


/* lame_encode_flush_nogap() */
Handle<Value> node_lame_encode_flush_nogap (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);
  Local<Object> outbuf = args[1]->ToObject();
  unsigned char *mp3buf = (unsigned char *)Buffer::Data(outbuf);
  int mp3buf_size = Buffer::Length(outbuf);

  int b = lame_encode_flush_nogap(gfp, mp3buf, mp3buf_size);
  return scope.Close(Integer::New(b));
}


/* lame_get_id3v1_tag() */
Handle<Value> node_lame_get_id3v1_tag (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  Local<Object> outbuf = args[1]->ToObject();
  unsigned char *buf = (unsigned char *)Buffer::Data(outbuf);
  size_t buf_size = (size_t)Buffer::Length(outbuf);

  size_t b = lame_get_id3v1_tag(gfp, buf, buf_size);
  return scope.Close(Integer::New(b));
}


/* lame_get_num_channels(gfp) */
Handle<Value> node_lame_get_num_channels (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  return scope.Close(Integer::New(lame_get_num_channels(gfp)));
}


/* lame_set_num_channels(gfp) */
Handle<Value> node_lame_set_num_channels (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  Local<Number> val = args[1]->ToNumber();
  return scope.Close(Integer::New(lame_set_num_channels(gfp, val->Int32Value())));
}


/* lame_init_params(gfp) */
Handle<Value> node_lame_init_params (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  if (lame_init_params(gfp) < 0) {
    return ThrowException(String::New("lame_init_params() failed"));
  }
  return Undefined();
}


/* lame_print_internals() */
Handle<Value> node_lame_print_internals (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  lame_print_internals(gfp);
  return Undefined();
}


/* lame_print_config() */
Handle<Value> node_lame_print_config (const Arguments& args) {
  HandleScope scope;
  // TODO: Argument validation
  Local<Object> wrapper = args[0]->ToObject();
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0);

  lame_print_config(gfp);
  return Undefined();
}


void Initialize(Handle<Object> target) {
  HandleScope scope;

  gfpClass = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
  gfpClass->SetInternalFieldCount(1);

  // Constants
  target->Set(String::New("MAXALBUMART"), Integer::New(LAME_MAXALBUMART), static_cast<PropertyAttribute>(ReadOnly|DontDelete));
  target->Set(String::New("MAXMP3BUFFER"), Integer::New(LAME_MAXMP3BUFFER), static_cast<PropertyAttribute>(ReadOnly|DontDelete));

  NODE_SET_METHOD(target, "get_lame_version", node_get_lame_version);
  NODE_SET_METHOD(target, "lame_close", node_lame_close);
  NODE_SET_METHOD(target, "lame_encode_buffer_interleaved", node_lame_encode_buffer_interleaved);
  NODE_SET_METHOD(target, "lame_encode_flush_nogap", node_lame_encode_flush_nogap);
  NODE_SET_METHOD(target, "lame_get_id3v1_tag", node_lame_get_id3v1_tag);
  NODE_SET_METHOD(target, "lame_get_num_channels", node_lame_get_num_channels);
  NODE_SET_METHOD(target, "lame_set_num_channels", node_lame_set_num_channels);
  NODE_SET_METHOD(target, "lame_init_params", node_lame_init_params);
  NODE_SET_METHOD(target, "lame_print_config", node_lame_print_config);
  NODE_SET_METHOD(target, "lame_print_internals", node_lame_print_internals);
  NODE_SET_METHOD(target, "malloc_gfp", node_malloc_gfp);

}

} // anonymous namespace

NODE_MODULE(nodelame, Initialize);
