var bindings = require('./build/default/nodelame.node')
  , inherits = require('util').inherits
  , Stream = require('stream').Stream

// XXX: Probably remove this line
exports = module.exports = bindings;

// export the raw bindings for test cases, etc.
exports.bindings = bindings;


// Config base class
function Config () {
  Stream.call(this);
  this._gfp = bindings.malloc_gfp();
  this._backlog = [];

  this.readable = this.writable = true;
  this.channels = 2;
  this.sampleSize = 16;
  this.sampleRate = 44100;
  this.BLOCK_ALIGN = this.sampleSize / 8 * this.channels // Number of 'Bytes per Sample'
  this.BYTES_PER_SECOND = this.sampleRate * this.BLOCK_ALIGN;
}
inherits(Config, Stream);
exports.Config = Config;

Config.prototype.close = function close (callback) {
  return bindings.lame_close(this._gfp, callback);
}

Config.prototype.getID3v1 = function getID3v1 (buffer, callback) {
  return bindings.lame_get_id3v1_tag(this._gfp, buffer, callback);
}

Config.prototype.initParams = function initParams () {
  this._init = true;
  return bindings.lame_init_params(this._gfp);
}


Config.prototype.printConfig = function printConfig () {
  return bindings.lame_print_config(this._gfp);
}

Config.prototype.printInternals = function printInternals () {
  return bindings.lame_print_internals(this._gfp);
}


// Encoder class
function Encoder (opts) {
  Config.call(this);
}
inherits(Encoder, Config);
exports.Encoder = Encoder;

// write raw PCM data
Encoder.prototype.write = function write (b) {
  if (!this._init) this.initParams();

  //console.error('Encoder#write');
  var l = b.length
    , num = ~~(l/this.BLOCK_ALIGN)
    , chunk = b.slice(0, num * this.BLOCK_ALIGN)
    , outbuf = new Buffer(1.25 * num)

  if (chunk.length != l) {
    this._backlog.push(b.slice(chunk.length));
    //console.error('pushing to backlog: %d bytes', this._backlog[this._backlog.length-1].length)
  }

  //console.error('beginning to encode %d bytes, %d samples', chunk.length, num);
  var b = bindings.lame_encode_buffer_interleaved(this._gfp, chunk, num, outbuf)
  //console.error('encoded %d bytes', b);

  this.emit('data', outbuf.slice(0, b));
  return true;
}

// flush the remaining
Encoder.prototype.end = function end () {
  //console.error('Encoder#end');
  var buf = new Buffer(9200);
  var b = bindings.lame_encode_flush_nogap(this._gfp, buf);
  this.emit('data', buf.slice(0, b));
  this.emit('end');
}

function createEncoder (opts) {
  return new Encoder(opts);
}
exports.createEncoder = exports.createEncoderStream = createEncoder;
