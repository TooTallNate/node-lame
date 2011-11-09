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


/* malloc()'s a `lame_t` struct and returns it to JS land */
Handle<Value> node_malloc_gfp (const Arguments& args) {
  HandleScope scope;

  lame_global_flags *gfp = lame_init();

  Local<Object> wrapper = gfpClass->NewInstance();
  wrapper->SetPointerInInternalField(0, gfp);

  return scope.Close(wrapper);
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

  if (lame_init_params(gfp) == -1) {
    return ThrowException(String::New("lame_init_params() failed"));
  }
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

  NODE_SET_METHOD(target, "get_lame_version", node_get_lame_version);
  NODE_SET_METHOD(target, "lame_get_num_channels", node_lame_get_num_channels);
  NODE_SET_METHOD(target, "lame_set_num_channels", node_lame_set_num_channels);
  NODE_SET_METHOD(target, "lame_init_params", node_lame_init_params);
  NODE_SET_METHOD(target, "lame_print_config", node_lame_print_config);
  NODE_SET_METHOD(target, "malloc_gfp", node_malloc_gfp);

}

} // anonymous namespace

NODE_MODULE(nodelame, Initialize);
