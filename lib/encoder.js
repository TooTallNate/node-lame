
/**
 * `Encoder` Stream class.
 * Encodes raw PCM data into an MP3 file.
 */

var bindings = require('./bindings')
  , inherits = require('util').inherits
  , Config = require('./config').Config


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
 */

function Encoder (opts) {
  Config.call(this);

  // disable the bit reservoir
  this.bitReservoir = false;
}
inherits(Encoder, Config);
exports.Encoder = Encoder;

Encoder.prototype.write = function write (b) {
  if (!this._init) this.initParams();

  //console.error('Encoder#write');
  var l = b.length
    , num = ~~(l/this.BLOCK_ALIGN)
    , chunk = b.slice(0, num * this.BLOCK_ALIGN)
    , outbuf = new Buffer(1.25 * num)

  if (chunk.length != l) {
    this._backlog.push(b.slice(chunk.length));
    console.error('pushing to backlog: %d bytes', this._backlog[this._backlog.length-1].length)
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
