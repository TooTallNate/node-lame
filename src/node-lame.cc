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

#define UNWRAP_GFP \
  HandleScope scope; \
  Local<Object> wrapper = args[0]->ToObject(); \
  lame_global_flags *gfp = (lame_global_flags *)wrapper->GetPointerFromInternalField(0); \


/* Wrapper ObjectTemplate to hold `lame_t` instances */
Persistent<ObjectTemplate> gfpClass;

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


/* malloc()'s a `lame_t` struct and returns it to JS land */
Handle<Value> node_lame_init (const Arguments& args) {
  HandleScope scope;

  lame_global_flags *gfp = lame_init();

  // disable id3v2 in the encode stream;
  // user must call lame_getid3v2_tag() manually
  // TODO: Move to it's own binding function
  lame_set_write_id3tag_automatic(gfp, 0);

  Local<Object> wrapper = gfpClass->NewInstance();
  wrapper->SetPointerInInternalField(0, gfp);

  return scope.Close(wrapper);
}

/* encode a buffer on the thread pool. */
void
EIO_encode_buffer_interleaved (uv_work_t *req) {
  encode_req *r = (encode_req *)req->data;
  r->rtn = lame_encode_buffer_interleaved(
    r->gfp,
    (short int *)r->input,
    r->num_samples,
    r->output,
    r->output_size
  );
}

void
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
  delete req;
}


/* lame_encode_buffer_interleaved()
 * The main encoding function */
Handle<Value> node_lame_encode_buffer_interleaved (const Arguments& args) {
  UNWRAP_GFP;

  // the input buffer
  char *input = Buffer::Data(args[1]->ToObject());
  int num_samples = args[2]->Int32Value();

  // the output buffer
  Local<Object> outbuf = args[3]->ToObject();
  int out_offset = args[4]->Int32Value();
  char *output = Buffer::Data(outbuf) + out_offset;
  int output_size = args[5]->Int32Value();

  encode_req *request = new encode_req;
  request->gfp = gfp;
  request->input = (unsigned char *)input;
  request->num_samples = num_samples;
  request->output = (unsigned char *)output;
  request->output_size = output_size;
  request->callback = Persistent<Function>::New(Local<Function>::Cast(args[6]));

  uv_work_t *req = new uv_work_t();
  req->data = request;
  uv_queue_work(uv_default_loop(),
                req,
                EIO_encode_buffer_interleaved,
                EIO_encode_buffer_interleaved_AFTER);
  return Undefined();
}


void
EIO_encode_flush_nogap (uv_work_t *req) {
  encode_req *r = (encode_req *)req->data;
  r->rtn = lame_encode_flush_nogap(
    r->gfp,
    r->output,
    r->output_size
  );
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

  uv_work_t *req = new uv_work_t();
  req->data = r;
  uv_queue_work(uv_default_loop(),
                req,
                EIO_encode_flush_nogap,
                EIO_encode_buffer_interleaved_AFTER);
  return Undefined();
}


/* lame_get_brate() */
Handle<Value> node_lame_get_brate (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_brate(gfp)));
}


/* lame_set_brate() */
Handle<Value> node_lame_set_brate (const Arguments& args) {
  UNWRAP_GFP;
  int val = args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_brate(gfp, val)));
}


/* lame_get_disable_reservoir() */
Handle<Value> node_lame_get_disable_reservoir (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_disable_reservoir(gfp)));
}


/* lame_set_disable_reservoir() */
Handle<Value> node_lame_set_disable_reservoir (const Arguments& args) {
  UNWRAP_GFP;
  int val = args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_disable_reservoir(gfp, val)));
}


/* lame_get_frameNum() */
Handle<Value> node_lame_get_frameNum (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_frameNum(gfp)));
}


/* lame_get_framesize() */
Handle<Value> node_lame_get_framesize (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_framesize(gfp)));
}


/* lame_get_id3v1_tag()
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


/* lame_get_id3v2_tag()
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


/* lame_get_in_samplerate(gfp) */
Handle<Value> node_lame_get_in_samplerate (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_in_samplerate(gfp)));
}


/* lame_set_set_in_samplerate(gfp) */
Handle<Value> node_lame_set_in_samplerate (const Arguments& args) {
  UNWRAP_GFP;
  int val = args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_in_samplerate(gfp, val)));
}


/* lame_get_num_channels(gfp) */
Handle<Value> node_lame_get_num_channels (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_num_channels(gfp)));
}


/* lame_set_num_channels(gfp) */
Handle<Value> node_lame_set_num_channels (const Arguments& args) {
  UNWRAP_GFP;
  int val = args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_num_channels(gfp, val)));
}


/* lame_get_out_samplerate(gfp) */
Handle<Value> node_lame_get_out_samplerate (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_out_samplerate(gfp)));
}


/* lame_set_out_samplerate(gfp) */
Handle<Value> node_lame_set_out_samplerate (const Arguments& args) {
  UNWRAP_GFP;
  int val = args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_out_samplerate(gfp, val)));
}


/* lame_get_version(gfp)
   version  0=MPEG-2  1=MPEG-1  (2=MPEG-2.5)     */
Handle<Value> node_lame_get_version (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_version(gfp)));
}


/* lame_get_VBR(gfp) */
Handle<Value> node_lame_get_VBR (const Arguments& args) {
  UNWRAP_GFP;
  return scope.Close(Integer::New(lame_get_VBR(gfp)));
}


/* lame_set_VBR(gfp) */
Handle<Value> node_lame_set_VBR (const Arguments& args) {
  UNWRAP_GFP;
  vbr_mode mode = (vbr_mode)args[1]->Int32Value();
  return scope.Close(Integer::New(lame_set_VBR(gfp, mode)));
}


/* lame_init_params(gfp) */
Handle<Value> node_lame_init_params (const Arguments& args) {
  UNWRAP_GFP;
  if (lame_init_params(gfp) < 0) {
    return ThrowException(String::New("lame_init_params() failed"));
  }
  return Undefined();
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


void Initialize(Handle<Object> target) {
  HandleScope scope;

  gfpClass = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
  gfpClass->SetInternalFieldCount(1);

  // Constants
  PropertyAttribute readonlydontdelete = static_cast<PropertyAttribute>(ReadOnly|DontDelete);
  // vbr_mode_e
  target->Set(String::New("vbr_off"), Integer::New(vbr_off), readonlydontdelete);
  target->Set(String::New("vbr_mt"), Integer::New(vbr_mt), readonlydontdelete);
  target->Set(String::New("vbr_rh"), Integer::New(vbr_rh), static_cast<PropertyAttribute>(ReadOnly|DontDelete));
  target->Set(String::New("vbr_abr"), Integer::New(vbr_abr), static_cast<PropertyAttribute>(ReadOnly|DontDelete));
  target->Set(String::New("vbr_mtrh"), Integer::New(vbr_mtrh), static_cast<PropertyAttribute>(ReadOnly|DontDelete));
  target->Set(String::New("vbr_default"), Integer::New(vbr_default), static_cast<PropertyAttribute>(ReadOnly|DontDelete));

  // Maximum size of an album art
  target->Set(String::New("MAXALBUMART"), Integer::New(LAME_MAXALBUMART), static_cast<PropertyAttribute>(ReadOnly|DontDelete));
  // This is deprecated I think...
  target->Set(String::New("MAXMP3BUFFER"), Integer::New(LAME_MAXMP3BUFFER), static_cast<PropertyAttribute>(ReadOnly|DontDelete));


  // Functions
  NODE_SET_METHOD(target, "get_lame_version", node_get_lame_version);
  NODE_SET_METHOD(target, "lame_close", node_lame_close);
  NODE_SET_METHOD(target, "lame_encode_buffer_interleaved", node_lame_encode_buffer_interleaved);
  NODE_SET_METHOD(target, "lame_encode_flush_nogap", node_lame_encode_flush_nogap);
  NODE_SET_METHOD(target, "lame_get_brate", node_lame_get_brate);
  NODE_SET_METHOD(target, "lame_set_brate", node_lame_set_brate);
  NODE_SET_METHOD(target, "lame_get_disable_reservoir", node_lame_get_disable_reservoir);
  NODE_SET_METHOD(target, "lame_set_disable_reservoir", node_lame_set_disable_reservoir);
  NODE_SET_METHOD(target, "lame_get_framesize", node_lame_get_framesize);
  NODE_SET_METHOD(target, "lame_get_frameNum", node_lame_get_frameNum);
  NODE_SET_METHOD(target, "lame_get_id3v1_tag", node_lame_get_id3v1_tag);
  NODE_SET_METHOD(target, "lame_get_id3v2_tag", node_lame_get_id3v2_tag);
  NODE_SET_METHOD(target, "lame_get_in_samplerate", node_lame_get_in_samplerate);
  NODE_SET_METHOD(target, "lame_set_in_samplerate", node_lame_set_in_samplerate);
  NODE_SET_METHOD(target, "lame_get_num_channels", node_lame_get_num_channels);
  NODE_SET_METHOD(target, "lame_set_num_channels", node_lame_set_num_channels);
  NODE_SET_METHOD(target, "lame_get_out_samplerate", node_lame_get_out_samplerate);
  NODE_SET_METHOD(target, "lame_set_out_samplerate", node_lame_set_out_samplerate);
  NODE_SET_METHOD(target, "lame_get_version", node_lame_get_version);
  NODE_SET_METHOD(target, "lame_get_VBR", node_lame_get_VBR);
  NODE_SET_METHOD(target, "lame_set_VBR", node_lame_set_VBR);
  NODE_SET_METHOD(target, "lame_init_params", node_lame_init_params);
  NODE_SET_METHOD(target, "lame_print_config", node_lame_print_config);
  NODE_SET_METHOD(target, "lame_print_internals", node_lame_print_internals);
  NODE_SET_METHOD(target, "lame_init", node_lame_init);

}

} // anonymous namespace

NODE_MODULE(nodelame, Initialize);
