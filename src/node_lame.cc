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
#include "lame.h"

#include "node_async_shim.h"

using namespace v8;
using namespace node;

namespace nodelame {

#define PASTE2(a, b) a##b
#define PASTE(a, b) PASTE2(a, b)

#define UNWRAP_GFP \
  HandleScope scope; \
  lame_global_flags *gfp = reinterpret_cast<lame_global_flags *>(Buffer::Data(args[0].As<Object>()));

#define FN(type, v8type, fn) \
Handle<Value> PASTE(node_lame_get_, fn) (const Arguments& args) { \
  UNWRAP_GFP; \
  type output = PASTE(lame_get_, fn)(gfp); \
  return scope.Close(Number::New(output)); \
} \
Handle<Value> PASTE(node_lame_set_, fn) (const Arguments& args) { \
  UNWRAP_GFP; \
  type input = (type)args[1]->PASTE(v8type, Value)(); \
  int output = PASTE(lame_set_, fn)(gfp, input); \
  return scope.Close(Number::New(output)); \
}


/* struct that's used for async encoding */
struct encode_req {
  lame_global_flags *gfp;
  unsigned char *input;
  int num_samples;
  unsigned char *output;
  int output_size;
  int rtn;
  Persistent<Function> callback;
};


/*
 * Called when the wrapped pointer is garbage collected.
 * We never have to do anything here...
 */

void wrap_pointer_cb(char *data, void *hint) {
  //fprintf(stderr, "wrap_pointer_cb\n");
}

Handle<Value> WrapPointer(char *ptr, size_t length) {
  HandleScope scope;
  void *user_data = NULL;
  Buffer *buf = Buffer::New(ptr, length, wrap_pointer_cb, user_data);
  return scope.Close(buf->handle_);
}

Handle<Value> WrapPointer(char *ptr) {
  size_t size = 0;
  return WrapPointer(ptr, size);
}


/* get_lame_version() */
Handle<Value> node_get_lame_version (const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New(get_lame_version()));
}

/* lame_close() */
Handle<Value> node_lame_close (const Arguments& args) {
  UNWRAP_GFP;
  lame_close(gfp);
  return Undefined();
}

/* called when a 'gfp' wrapper Object get's GC'd from JS-land */
/*
void gfp_weak_callback (Persistent<Value> wrapper, void *arg) {
  HandleScope scope;
  lame_global_flags *gfp = (lame_global_flags *)arg;
  lame_close(gfp);
  wrapper.Dispose();
}
*/

/* malloc()'s a `lame_t` struct and returns it to JS land */
Handle<Value> node_lame_init (const Arguments& args) {
  HandleScope scope;

  lame_global_flags *gfp = lame_init();
  if (gfp == NULL) return Null();

  // disable id3v2 in the encode stream;
  // user must call lame_getid3v2_tag() manually
  // TODO: Move to it's own binding function
  lame_set_write_id3tag_automatic(gfp, 0);

  Handle<Value> wrapper = WrapPointer((char *)gfp);
  return scope.Close(wrapper);
}


/* encode a buffer on the thread pool. */
async_rtn
EIO_encode_buffer_interleaved (uv_work_t *req) {
  encode_req *r = (encode_req *)req->data;
  r->rtn = lame_encode_buffer_interleaved(
    r->gfp,
    (short int *)r->input,
    r->num_samples,
    r->output,
    r->output_size
  );
  RETURN_ASYNC;
}

async_rtn
EIO_encode_buffer_interleaved_AFTER (uv_work_t *req) {
  HandleScope scope;

  encode_req *r = (encode_req *)req->data;

  Handle<Value> argv[1];
  argv[0] = Integer::New(r->rtn);

  TryCatch try_catch;

  r->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  if (try_catch.HasCaught())
    FatalException(try_catch);

  // cleanup
  r->callback.Dispose();
  delete r;
  RETURN_ASYNC_AFTER;
}


/* lame_encode_buffer_interleaved()
 * The main encoding function */
Handle<Value> node_lame_encode_buffer_interleaved (const Arguments& args) {
  UNWRAP_GFP;

  // the input buffer
  char *input = Buffer::Data(args[1].As<Object>());
  int num_samples = args[2]->Int32Value();

  // the output buffer
  int out_offset = args[4]->Int32Value();
  char *output = Buffer::Data(args[3].As<Object>()) + out_offset;
  int output_size = args[5]->Int32Value();

  encode_req *request = new encode_req;
  request->gfp = gfp;
  request->input = (unsigned char *)input;
  request->num_samples = num_samples;
  request->output = (unsigned char *)output;
  request->output_size = output_size;
  request->callback = Persistent<Function>::New(Local<Function>::Cast(args[6]));

  BEGIN_ASYNC(request, EIO_encode_buffer_interleaved, EIO_encode_buffer_interleaved_AFTER);
  return Undefined();
}


async_rtn
EIO_encode_flush_nogap (uv_work_t *req) {
  encode_req *r = (encode_req *)req->data;
  r->rtn = lame_encode_flush_nogap(
    r->gfp,
    r->output,
    r->output_size
  );
  RETURN_ASYNC;
}

/* lame_encode_flush_nogap() */
Handle<Value> node_lame_encode_flush_nogap (const Arguments& args) {
  UNWRAP_GFP;

  // the output buffer
  Local<Object> outbuf = args[1]->ToObject();
  int out_offset = args[2]->Int32Value();
  char *output = Buffer::Data(outbuf) + out_offset;
  int output_size = args[3]->Int32Value();

  encode_req *r = new encode_req;
  r->gfp = gfp;
  r->output = (unsigned char *)output;
  r->output_size = output_size;
  r->callback = Persistent<Function>::New(Local<Function>::Cast(args[4]));

  BEGIN_ASYNC(r, EIO_encode_flush_nogap, EIO_encode_buffer_interleaved_AFTER);
  return Undefined();
}



/**
 * lame_get_id3v1_tag()
 * Must be called *after* lame_encode_flush()
 * TODO: Make async
 */
Handle<Value> node_lame_get_id3v1_tag (const Arguments& args) {
  UNWRAP_GFP;

  Local<Object> outbuf = args[1]->ToObject();
  unsigned char *buf = (unsigned char *)Buffer::Data(outbuf);
  size_t buf_size = (size_t)Buffer::Length(outbuf);

  size_t b = lame_get_id3v1_tag(gfp, buf, buf_size);
  return scope.Close(Integer::New(b));
}


/**
 * lame_get_id3v2_tag()
 * Must be called *before* lame_init_params()
 * TODO: Make async
 */
Handle<Value> node_lame_get_id3v2_tag (const Arguments& args) {
  UNWRAP_GFP;

  Local<Object> outbuf = args[1]->ToObject();
  unsigned char *buf = (unsigned char *)Buffer::Data(outbuf);
  size_t buf_size = (size_t)Buffer::Length(outbuf);

  size_t b = lame_get_id3v2_tag(gfp, buf, buf_size);
  return scope.Close(Integer::New(b));
}


/* lame_init_params(gfp) */
Handle<Value> node_lame_init_params (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Number::New(lame_init_params(gfp)));
}


/* lame_print_internals() */
Handle<Value> node_lame_print_internals (const Arguments& args) {
  UNWRAP_GFP;
  lame_print_internals(gfp);
  return Undefined();
}


/* lame_print_config() */
Handle<Value> node_lame_print_config (const Arguments& args) {
  UNWRAP_GFP;
  lame_print_config(gfp);
  return Undefined();
}

// define the node_lame_get/node_lame_set functions
FN(unsigned long, Number, num_samples);
FN(int, Int32, in_samplerate);
FN(int, Int32, num_channels);
FN(float, Number, scale);
FN(float, Number, scale_left);
FN(float, Number, scale_right);
FN(int, Int32, out_samplerate);
FN(int, Int32, analysis);
FN(int, Int32, bWriteVbrTag);
FN(int, Int32, quality);
FN(MPEG_mode, Int32, mode);

FN(int, Int32, brate);
FN(float, Number, compression_ratio);
FN(int, Int32, copyright);
FN(int, Int32, original);
FN(int, Int32, error_protection);
FN(int, Int32, extension);
FN(int, Int32, strict_ISO);
FN(int, Int32, disable_reservoir);
FN(int, Int32, quant_comp);
FN(int, Int32, quant_comp_short);
FN(int, Int32, exp_nspsytune);
FN(vbr_mode, Int32, VBR);
FN(int, Int32, VBR_q);
FN(float, Number, VBR_quality);
FN(int, Int32, VBR_mean_bitrate_kbps);
FN(int, Int32, VBR_min_bitrate_kbps);
FN(int, Int32, VBR_max_bitrate_kbps);
FN(int, Int32, VBR_hard_min);
FN(int, Int32, lowpassfreq);
FN(int, Int32, lowpasswidth);
FN(int, Int32, highpassfreq);
FN(int, Int32, highpasswidth);
// ...


void InitLame(Handle<Object> target) {
  HandleScope scope;

#define CONST_INT(value) \
  target->Set(String::NewSymbol(#value), Integer::New(value), \
      static_cast<PropertyAttribute>(ReadOnly|DontDelete));

  // vbr_mode_e
  CONST_INT(vbr_off);
  CONST_INT(vbr_mt);
  CONST_INT(vbr_rh);
  CONST_INT(vbr_abr);
  CONST_INT(vbr_mtrh);
  CONST_INT(vbr_default);
  // MPEG_mode_e
  CONST_INT(STEREO);
  CONST_INT(JOINT_STEREO);
  CONST_INT(MONO);
  CONST_INT(NOT_SET);
  // Padding_type_e
  CONST_INT(PAD_NO);
  CONST_INT(PAD_ALL);
  CONST_INT(PAD_ADJUST);
  // Maximum size of an album art
  CONST_INT(LAME_MAXALBUMART);


  // Functions
  NODE_SET_METHOD(target, "get_lame_version", node_get_lame_version);
  NODE_SET_METHOD(target, "lame_close", node_lame_close);
  NODE_SET_METHOD(target, "lame_encode_buffer_interleaved", node_lame_encode_buffer_interleaved);
  NODE_SET_METHOD(target, "lame_encode_flush_nogap", node_lame_encode_flush_nogap);
  NODE_SET_METHOD(target, "lame_get_id3v1_tag", node_lame_get_id3v1_tag);
  NODE_SET_METHOD(target, "lame_get_id3v2_tag", node_lame_get_id3v2_tag);
  NODE_SET_METHOD(target, "lame_init_params", node_lame_init_params);
  NODE_SET_METHOD(target, "lame_print_config", node_lame_print_config);
  NODE_SET_METHOD(target, "lame_print_internals", node_lame_print_internals);
  NODE_SET_METHOD(target, "lame_init", node_lame_init);

  // Get/Set functions
#define LAME_SET_METHOD(fn) \
  NODE_SET_METHOD(target, "lame_get_" #fn, PASTE(node_lame_get_, fn)); \
  NODE_SET_METHOD(target, "lame_set_" #fn, PASTE(node_lame_set_, fn));

  LAME_SET_METHOD(num_samples);
  LAME_SET_METHOD(in_samplerate);
  LAME_SET_METHOD(num_channels);
  LAME_SET_METHOD(scale);
  LAME_SET_METHOD(scale_left);
  LAME_SET_METHOD(scale_right);
  LAME_SET_METHOD(out_samplerate);
  LAME_SET_METHOD(analysis);
  LAME_SET_METHOD(bWriteVbrTag);
  LAME_SET_METHOD(quality);
  LAME_SET_METHOD(mode);

  LAME_SET_METHOD(brate);
  LAME_SET_METHOD(compression_ratio);
  LAME_SET_METHOD(copyright);
  LAME_SET_METHOD(original);
  LAME_SET_METHOD(error_protection);
  LAME_SET_METHOD(extension);
  LAME_SET_METHOD(strict_ISO);
  LAME_SET_METHOD(disable_reservoir);
  LAME_SET_METHOD(quant_comp);
  LAME_SET_METHOD(quant_comp_short);
  LAME_SET_METHOD(exp_nspsytune);
  LAME_SET_METHOD(VBR);
  LAME_SET_METHOD(VBR_q);
  LAME_SET_METHOD(VBR_quality);
  LAME_SET_METHOD(VBR_mean_bitrate_kbps);
  LAME_SET_METHOD(VBR_min_bitrate_kbps);
  LAME_SET_METHOD(VBR_max_bitrate_kbps);
  LAME_SET_METHOD(VBR_hard_min);
  LAME_SET_METHOD(lowpassfreq);
  LAME_SET_METHOD(lowpasswidth);
  LAME_SET_METHOD(highpassfreq);
  LAME_SET_METHOD(highpasswidth);
  // ...

  /*
  NODE_SET_METHOD(target, "lame_get_decode_only", node_lame_get_decode_only);
  NODE_SET_METHOD(target, "lame_set_decode_only", node_lame_set_decode_only);
  NODE_SET_METHOD(target, "lame_get_framesize", node_lame_get_framesize);
  NODE_SET_METHOD(target, "lame_get_frameNum", node_lame_get_frameNum);
  NODE_SET_METHOD(target, "lame_get_version", node_lame_get_version);
  */

}

} // nodelame namespace
