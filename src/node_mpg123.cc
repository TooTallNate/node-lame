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
#include <mpg123.h>

using namespace v8;
using namespace node;

namespace nodelame {

#define UNWRAP_MH \
  HandleScope scope; \
  Local<Object>wrapper = args[0]->ToObject(); \
  mpg123_handle *mh = (mpg123_handle *)wrapper->GetPointerFromInternalField(0);

/* Wrapper ObjectTemplate to hold `mpg123_handle` instances */
Persistent<ObjectTemplate> mhClass;

Handle<Value> node_mpg123_init (const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(mpg123_init()));
}

Handle<Value> node_mpg123_exit (const Arguments& args) {
  HandleScope scope;
  mpg123_exit();
  return Undefined();
}

void mh_weak_callback (Persistent<Value> wrapper, void *arg) {
  HandleScope scope;
  mpg123_handle *mh = (mpg123_handle *)arg;
  mpg123_delete(mh);
  wrapper.Dispose();
}

Handle<Value> node_mpg123_new (const Arguments& args) {
  HandleScope scope;
  int error = 0;
  mpg123_handle *mh = mpg123_new(NULL, &error);

  Handle<Value> rtn;
  if (error == MPG123_OK) {
    Persistent<Object> o = Persistent<Object>::New(mhClass->NewInstance());
    o->SetPointerInInternalField(0, mh);
    o.MakeWeak(mh, mh_weak_callback);
    rtn = o;
  } else {
    rtn = Integer::New(error);
  }
  return scope.Close(rtn);
}

Handle<Value> node_mpg123_supported_decoders (const Arguments& args) {
  HandleScope scope;
  const char **decoders = mpg123_supported_decoders();
  int i = 0;
  Handle<Array> rtn = Array::New();
  while (*decoders != NULL) {
    rtn->Set(Integer::New(i++), String::New(*decoders));
    decoders++;
  }
  return scope.Close(rtn);
}

Handle<Value> node_mpg123_decoders (const Arguments& args) {
  HandleScope scope;
  const char **decoders = mpg123_decoders();
  int i = 0;
  Handle<Array> rtn = Array::New();
  while (*decoders != NULL) {
    rtn->Set(Integer::New(i++), String::New(*decoders));
    decoders++;
  }
  return scope.Close(rtn);
}

// TODO: Make async
Handle<Value> node_mpg123_open_feed (const Arguments& args) {
  UNWRAP_MH;
  int ret = mpg123_open_feed(mh);
  return scope.Close(Integer::New(ret));
}

// TODO: Make async
Handle<Value> node_mpg123_decode (const Arguments& args) {
  UNWRAP_MH;

  // input MP3 data
  const unsigned char *input = (const unsigned char *)Buffer::Data(args[1]->ToObject());
  size_t insize = args[2]->Int32Value();

  // the output buffer
  Local<Object> outbuf = args[3]->ToObject();
  int out_offset = args[4]->Int32Value();
  unsigned char *output = (unsigned char *)(Buffer::Data(outbuf) + out_offset);
  size_t outsize = args[5]->Int32Value();

  size_t size = 0;
  int ret = mpg123_decode(mh, input, insize, output, outsize, &size);

  Local<Object> rtn = Object::New();
  rtn->Set(String::NewSymbol("ret"), Integer::New(ret));
  rtn->Set(String::NewSymbol("size"), Integer::New(size));
  return scope.Close(rtn);
}


void InitMPG123(Handle<Object> target) {
  HandleScope scope;

  mhClass = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
  mhClass->SetInternalFieldCount(1);

  NODE_SET_METHOD(target, "mpg123_init", node_mpg123_init);
  NODE_SET_METHOD(target, "mpg123_exit", node_mpg123_exit);
  NODE_SET_METHOD(target, "mpg123_new", node_mpg123_new);
  NODE_SET_METHOD(target, "mpg123_decoders", node_mpg123_decoders);
  NODE_SET_METHOD(target, "mpg123_supported_decoders", node_mpg123_supported_decoders);
  NODE_SET_METHOD(target, "mpg123_open_feed", node_mpg123_open_feed);
  NODE_SET_METHOD(target, "mpg123_decode", node_mpg123_decode);
}

} // nodelame namespace
