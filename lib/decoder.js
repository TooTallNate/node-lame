
/**
 * `Decoder` Stream class.
 *  Accepts an MP3 file and spits out raw PCM data.
 *
 *  http://sourceforge.net/mailarchive/message.php?msg_id=26907120
 *
 *  Not thread safe :(
 *  http://sourceforge.net/mailarchive/message.php?msg_id=23774940
 */

var bindings = require('./bindings')
  , inherits = require('util').inherits
  , Config = require('./config').Config
  , assert = require('assert')

exports.createDecoder = function createDecoder (opts) {
  return new Decoder (opts);
}

function Decoder (opts) {
  Config.call(this, opts);

  // tell LAME that this is a decoder instance
  this.decoding = true;
}
inherits(Decoder, Config);
exports.Decoder = Decoder;
