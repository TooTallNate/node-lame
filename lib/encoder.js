
/**
 * `Encoder` Stream class.
 * Encodes raw PCM data into a MP3 file.
 */

var bindings = require('./bindings')
  , inherits = require('util').inherits
  , Config = require('./config').Config
  , assert = require('assert')


/**
 * Creates a new Encoder instance.
 */

function createEncoder (opts) {
  return new Encoder(opts);
}
exports.createEncoder = exports.createEncoderStream = createEncoder;


/**
 * `Encoder` class is a readable and writable Stream class.
 * Write raw PCM data, out comes an MP3 file.
 * A lot of this logic is reused from the zlib module.
 */

function Encoder (opts) {
  Config.call(this, opts);

  // disable the bit reservoir
  this.bitReservoir = false;
}
inherits(Encoder, Config);
exports.Encoder = Encoder;

Encoder.prototype.write = function write (chunk, cb) {
  if (this._ended) {
    return this.emit('error', new Error('Cannot write after end'));
  }
  if (!this._init) this._initParams();

  if (arguments.length === 1 && typeof chunk === 'function') {
    cb = chunk;
    chunk = null;
  }

  var empty = this._queue.length === 0;

  this._queue.push([chunk, cb]);
  this._process();
  if (!empty) {
    this._needDrain = true;
  }
  return empty;
}

// flush the remaining
Encoder.prototype.end = function end (chunk, cb) {
  var self = this;
  this._ending = true;
  // flush the remaining MP3 data in LAME's buffers
  var ret = this.write(chunk, function () {
    self.emit('end');
    if (cb) cb();
  });
  this._ended = true;
  return ret;
}

Encoder.prototype._process = function _process () {
  if (this._processing || this._paused) return;

  if (this._queue.length === 0) {
    if (this._needDrain) {
      this._needDrain = false;
      this.emit('drain');
    }
    // nothing to do, waiting for more data at this point.
    return;
  }

  var req = this._queue.shift()
    , self = this
    , cb = req.pop()
    , chunk = req.pop()
    , flushing = this._ending && this._queue.length === 0
    , availOutBefore = this._chunkSize - this._offset
    , num = 0
    , estimatedSize = 0;

  if (flushing) {
    estimatedSize = 7200;
  } else {
    num = chunk.length / this.BLOCK_ALIGN;
    // TODO: Use better calculation logic from lame.h here
    estimatedSize = 1.25 * num + 7200;

    // Make sure we got *whole* PCM frames
    assert.ok(Buffer.isBuffer(chunk));
    assert.equal(chunk.length % this.BLOCK_ALIGN, 0);
  }

  // Check to make sure out Buffer is big enough
  if (availOutBefore < estimatedSize) {
    //console.error('realloc _buffer');
    self._offset = 0;
    self._buffer = new Buffer(self._chunkSize);
    availOutBefore = self._chunkSize;
  }

  if (flushing) {
    // call lame flush function
    //console.error('invoke async encode flush');
    bindings.lame_encode_flush_nogap(this._gfp
                                   , this._buffer
                                   , this._offset
                                   , availOutBefore
                                   , afterWrite);
  } else {
    //console.error('invoke async encode:', chunk.length, num, this._offset, availOutBefore)
    bindings.lame_encode_buffer_interleaved(this._gfp // gfp
                                          , chunk     // pcm data
                                          , num       // # / samples / channel
                                          , this._buffer // out buffer
                                          , this._offset
                                          , availOutBefore
                                          , afterWrite);
  }
  this._processing = true;

  function afterWrite (bytesWritten) {
    //console.error('afterWrite:', bytesWritten)
    if (bytesWritten === -1) {
      // out buffer too small!!!
      return self.emit('error', new Error('output buffer too small!'));
    } else if (bytesWritten === -2) {
      return self.emit('error', new Error('malloc() problems'));
    } else if (bytesWritten === -3) {
      return self.emit('error', new Error('lame_init_params() not called'));
    } else if (bytesWritten === -4) {
      return self.emit('error', new Error('phycho acoustic problems'));
    } else {
      var out = self._buffer.slice(self._offset, self._offset + bytesWritten);
      self._offset += bytesWritten;
      self.emit('data', out);
    }

    // finished with this chunk
    self._processing = false;
    if (cb) cb();
    self._process();
  }
}
