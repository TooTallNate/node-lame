
/**
 * Module dependencies.
 */

var assert = require('assert');
var binding = require('./bindings');
var inherits = require('util').inherits;
var Transform = require('stream').Transform;
var debug = require('debug')('lame:encoder');

/**
 * Module exports.
 */

module.exports = Encoder;

/**
 * Messages for error codes returned from the lame C encoding functions.
 */

var ERRORS = {
  '-1': 'output buffer too small',
  '-2': 'malloc() problems',
  '-3': 'lame_init_params() not called',
  '-4': 'psycho acoustic problems'
};


/**
 * The `Encoder` class is a Transform stream class.
 * Write raw PCM data, out comes an MP3 file.
 *
 * @param {Object} opts PCM stream info and stream options
 * @api public
 */

function Encoder (opts) {
  if (!(this instanceof Encoder)) {
    return new Encoder(opts);
  }
  Transform.call(this, opts);

  // lame malloc()s the "gfp" buffer
  this.gfp = binding.lame_init();

  // TODO: configurable...
  this.channels = 2;
  binding.lame_set_num_channels(this.gfp, this.channels);

  this.sampleSize = 16;

  this.sampleRate = 44100;
  binding.lame_set_in_samplerate(this.gfp, this.sampleRate);

  // constant: number of 'bytes per sample'
  this.blockAlign = this.sampleSize / 8 * this.channels;

  // disable the bit reservoir
  //this.bitReservoir = false;

  // XXX: defer calling this (allow additional properties to be set)?
  if (0 !== binding.lame_init_params(this.gfp)) {
    // TODO: throw error
  }
}
inherits(Encoder, Transform);

/**
 * Calls `lame_encode_buffer_interleaved()` on the given "chunk.
 *
 * @api private
 */

Encoder.prototype._transform = function (chunk, write, done) {
  debug('_transform (%d bytes)', chunk.length);

  // first handle any _remainder
  if (this._remainder) {
    debug('concating remainder');
    chunk = Buffer.concat([ this._remainder, chunk ]);
    this._remainder = null;
  }
  // set any necessary _remainder (we can only send whole samples at a time)
  var remainder = chunk.length % this.blockAlign;
  if (remainder > 0) {
    debug('setting remainder of %d bytes', remainder);
    var slice = chunk.length - remainder;
    this._remainder = chunk.slice(slice);
    chunk = chunk.slice(0, slice);
  }
  assert.equal(chunk.length % this.blockAlign, 0);

  var num_samples = chunk.length / this.blockAlign;
    // TODO: Use better calculation logic from lame.h here
  var estimated_size = 1.25 * num_samples + 7200;
  var output = new Buffer(estimated_size);
  debug('encoding %d byte chunk with %d byte output buffer (%d samples)', chunk.length, output.length, num_samples);

  binding.lame_encode_buffer_interleaved(
    this.gfp,
    chunk,
    num_samples,
    output,
    0,
    output.length,
    cb
  );

  function cb (bytesWritten) {
    debug('after lame_encode_buffer_interleaved() (rtn: %d)', bytesWritten);
    if (bytesWritten < 0) {
      var err = new Error(ERRORS[bytesWritten]);
      err.code = bytesWritten;
      done(err);
    } else if (bytesWritten > 0) {
      output = output.slice(0, bytesWritten);
      debug('writing %d MP3 bytes', output.length);
      write(output);
      done();
    } else { // bytesWritten == 0
      done();
    }
  }
};

/**
 * Calls `lame_encode_flush_nogap()` on the thread pool.
 */

Encoder.prototype._flush = function (write, done) {
  debug('_flush');
  var estimated_size = 7200; // value specified in lame.h
  var output = new Buffer(estimated_size);

  binding.lame_encode_flush_nogap(
    this.gfp,
    output,
    0,
    output.length,
    cb
  );

  function cb (bytesWritten) {
    debug('after lame_encode_flush_nogap() (rtn: %d)', bytesWritten);
    if (bytesWritten < 0) {
      var err = new Error(ERRORS[bytesWritten]);
      err.code = bytesWritten;
      done(err);
    } else if (bytesWritten > 0) {
      output = output.slice(0, bytesWritten);
      write(output);
      done();
    } else { // bytesWritten == 0
      done();
    }
  }
};
