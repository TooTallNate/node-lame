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

#include "node_async_shim.h"

using namespace v8;
using namespace node;

namespace nodelame {

#define UNWRAP_MH \
  HandleScope scope; \
  Local<Object>wrapper = args[0]->ToObject(); \
  mpg123_handle *mh = (mpg123_handle *)wrapper->GetPointerFromInternalField(0);

/* Wrapper ObjectTemplate to hold `mpg123_handle` instances */
Persistent<ObjectTemplate> mhClass;

/* struct used for async decoding */
struct decode_req {
  mpg123_handle *mh;
  const unsigned char *input;
  size_t insize;
  unsigned char *output;
  size_t outsize;
  size_t bytesWritten;
  int ret;
  Persistent<Function> callback;
};

struct new_format {
  long rate;
  int channels;
  int encoding;
};

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

// TODO: Make async
Handle<Value> node_mpg123_new (const Arguments& args) {
  HandleScope scope;

  // TODO: Accept an input decoder String
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

Handle<Value> node_mpg123_current_decoder (const Arguments& args) {
  UNWRAP_MH;
  const char *decoder = mpg123_current_decoder(mh);
  return scope.Close(String::New(decoder));
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
  const unsigned char *input = NULL;
  if (!args[1]->IsNull()) {
    input = (const unsigned char *)Buffer::Data(args[1]->ToObject());
  }
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

  if (ret == MPG123_NEW_FORMAT) {
    long rate;
    int channels;
    int encoding;
    mpg123_getformat(mh, &rate, &channels, &encoding);
    rtn->Set(String::NewSymbol("rate"), Integer::New(rate));
    rtn->Set(String::NewSymbol("channels"), Integer::New(channels));
    rtn->Set(String::NewSymbol("encoding"), Integer::New(encoding));
  }

  return scope.Close(rtn);
}


void InitMPG123(Handle<Object> target) {
  HandleScope scope;

  mhClass = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
  mhClass->SetInternalFieldCount(1);

#define CONST_INT(value) \
  target->Set(String::NewSymbol(#value), Integer::New(value), \
      static_cast<PropertyAttribute>(ReadOnly|DontDelete));

  // mpg123_errors
  CONST_INT(MPG123_DONE);  /**< Message: Track ended. Stop decoding. */
  CONST_INT(MPG123_NEW_FORMAT);  /**< Message: Output format will be different on next call. Note that some libmpg123 versions between 1.4.3 and 1.8.0 insist on you calling mpg123_getformat() after getting this message code. Newer verisons behave like advertised: You have the chance to call mpg123_getformat(), but you can also just continue decoding and get your data. */
  CONST_INT(MPG123_NEED_MORE);  /**< Message: For feed reader: "Feed me more!" (call mpg123_feed() or mpg123_decode() with some new input data). */
  CONST_INT(MPG123_ERR);      /**< Generic Error */
  CONST_INT(MPG123_OK);       /**< Success */
  CONST_INT(MPG123_BAD_OUTFORMAT);   /**< Unable to set up output format! */
  CONST_INT(MPG123_BAD_CHANNEL);    /**< Invalid channel number specified. */
  CONST_INT(MPG123_BAD_RATE);    /**< Invalid sample rate specified.  */
  CONST_INT(MPG123_ERR_16TO8TABLE);  /**< Unable to allocate memory for 16 to 8 converter table! */
  CONST_INT(MPG123_BAD_PARAM);    /**< Bad parameter id! */
  CONST_INT(MPG123_BAD_BUFFER);    /**< Bad buffer given -- invalid pointer or too small size. */
  CONST_INT(MPG123_OUT_OF_MEM);    /**< Out of memory -- some malloc() failed. */
  CONST_INT(MPG123_NOT_INITIALIZED);  /**< You didn't initialize the library! */
  CONST_INT(MPG123_BAD_DECODER);    /**< Invalid decoder choice. */
  CONST_INT(MPG123_BAD_HANDLE);    /**< Invalid mpg123 handle. */
  CONST_INT(MPG123_NO_BUFFERS);    /**< Unable to initialize frame buffers (out of memory?). */
  CONST_INT(MPG123_BAD_RVA);      /**< Invalid RVA mode. */
  CONST_INT(MPG123_NO_GAPLESS);    /**< This build doesn't support gapless decoding. */
  CONST_INT(MPG123_NO_SPACE);    /**< Not enough buffer space. */
  CONST_INT(MPG123_BAD_TYPES);    /**< Incompatible numeric data types. */
  CONST_INT(MPG123_BAD_BAND);    /**< Bad equalizer band. */
  CONST_INT(MPG123_ERR_NULL);    /**< Null pointer given where valid storage address needed. */
  CONST_INT(MPG123_ERR_READER);    /**< Error reading the stream. */
  CONST_INT(MPG123_NO_SEEK_FROM_END);/**< Cannot seek from end (end is not known). */
  CONST_INT(MPG123_BAD_WHENCE);    /**< Invalid 'whence' for seek function.*/
  CONST_INT(MPG123_NO_TIMEOUT);    /**< Build does not support stream timeouts. */
  CONST_INT(MPG123_BAD_FILE);    /**< File access error. */
  CONST_INT(MPG123_NO_SEEK);     /**< Seek not supported by stream. */
  CONST_INT(MPG123_NO_READER);    /**< No stream opened. */
  CONST_INT(MPG123_BAD_PARS);    /**< Bad parameter handle. */
  CONST_INT(MPG123_BAD_INDEX_PAR);  /**< Bad parameters to mpg123_index() and mpg123_set_index() */
  CONST_INT(MPG123_OUT_OF_SYNC);  /**< Lost track in bytestream and did not try to resync. */
  CONST_INT(MPG123_RESYNC_FAIL);  /**< Resync failed to find valid MPEG data. */
  CONST_INT(MPG123_NO_8BIT);  /**< No 8bit encoding possible. */
  CONST_INT(MPG123_BAD_ALIGN);  /**< Stack aligmnent error */
  CONST_INT(MPG123_NULL_BUFFER);  /**< NULL input buffer with non-zero size... */
  CONST_INT(MPG123_NO_RELSEEK);  /**< Relative seek not possible (screwed up file offset) */
  CONST_INT(MPG123_NULL_POINTER); /**< You gave a null pointer somewhere where you shouldn't have. */
  CONST_INT(MPG123_BAD_KEY);   /**< Bad key value given. */
  CONST_INT(MPG123_NO_INDEX);  /**< No frame index in this build. */
  CONST_INT(MPG123_INDEX_FAIL);  /**< Something with frame index went wrong. */
  CONST_INT(MPG123_BAD_DECODER_SETUP);  /**< Something prevents a proper decoder setup */
  CONST_INT(MPG123_MISSING_FEATURE);  /**< This feature has not been built into libmpg123. */
  CONST_INT(MPG123_BAD_VALUE); /**< A bad value has been given, somewhere. */
  CONST_INT(MPG123_LSEEK_FAILED); /**< Low-level seek failed. */
  CONST_INT(MPG123_BAD_CUSTOM_IO); /**< Custom I/O not prepared. */
  CONST_INT(MPG123_LFS_OVERFLOW); /**< Offset value overflow during translation of large file API calls -- your client program cannot handle that large file. */

  /* mpg123_channelcount */
  CONST_INT(MPG123_MONO);
  CONST_INT(MPG123_STEREO);

  // mpg123_channels
  CONST_INT(MPG123_LEFT);
  CONST_INT(MPG123_RIGHT);
  CONST_INT(MPG123_LR);

  CONST_INT(MPG123_ID3);
  CONST_INT(MPG123_NEW_ID3);
  CONST_INT(MPG123_ICY);
  CONST_INT(MPG123_NEW_ICY);

  NODE_SET_METHOD(target, "mpg123_init", node_mpg123_init);
  NODE_SET_METHOD(target, "mpg123_exit", node_mpg123_exit);
  NODE_SET_METHOD(target, "mpg123_new", node_mpg123_new);
  NODE_SET_METHOD(target, "mpg123_decoders", node_mpg123_decoders);
  NODE_SET_METHOD(target, "mpg123_current_decoder", node_mpg123_current_decoder);
  NODE_SET_METHOD(target, "mpg123_supported_decoders", node_mpg123_supported_decoders);
  NODE_SET_METHOD(target, "mpg123_open_feed", node_mpg123_open_feed);
  NODE_SET_METHOD(target, "mpg123_decode", node_mpg123_decode);
}

} // nodelame namespace
