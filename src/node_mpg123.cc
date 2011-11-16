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
#include <mpg123.h>

using namespace v8;
using namespace node;

namespace nodelame {

Handle<Value> node_mpg123_init (const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(mpg123_init()));
}

Handle<Value> node_mpg123_exit (const Arguments& args) {
  HandleScope scope;
  mpg123_exit();
  return Undefined();
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

void InitMPG123(Handle<Object> target) {
  HandleScope scope;

  NODE_SET_METHOD(target, "mpg123_init", node_mpg123_init);
  NODE_SET_METHOD(target, "mpg123_exit", node_mpg123_exit);
  NODE_SET_METHOD(target, "mpg123_decoders", node_mpg123_decoders);
}

} // nodelame namespace
