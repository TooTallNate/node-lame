
/**
 * Module dependencies.
 */
var Stream = require('stream').Stream
  , inherits = require('util').inherits
  , parse = require('./parse').parseFrameHeader
  , concat = require('./util').buffer_concat
  , strtok = require('strtok')
  , ID3V1_LENGTH = 128
  , ID3V2_HEADER_LENGTH = 10
  , MPEG_HEADER_LENGTH = 4

/**
 * Static BufferType instances.
 */

var MPEG_HEADER = new strtok.BufferType(MPEG_HEADER_LENGTH)
  , REST_OF_ID3V2_HEADER = new strtok.BufferType(ID3V2_HEADER_LENGTH - MPEG_HEADER_LENGTH)
  , REST_OF_ID3V1 = new strtok.BufferType(ID3V1_LENGTH - MPEG_HEADER_LENGTH);


/**
 * Creates a new Parser instance.
 */

exports.createParser = function createParser () {
  return new Parser();
}


/**
 * `Parser` class accepts an MP3 file and emits events as the structure of the
 * file is parsed. All events have the first argument as a Buffer of the piece
 * that has been parsed:
 *
 *  * 'header' events on each frame header.
 *  * 'frame' events on heach frame data.
 *  * 'id3v1' event on the ID3v1 data at the end (may not be called).
 *  * 'id3v2' event on the ID3v2 data at the beginning (may not be called).
 *
 * XXX: I guess this should theoretically work on
 * any MPEG file, I haven't tested yet.
 */

function Parser () {
  Stream.call(this);
  this.readable = this.writable = true;
  this.frameSize = -1;
  // true until the first MPEG_HEADER has been received
  this.beginning = true;

  var self = this;
  strtok.parse(this, function (v, cb) {
    // start things off
    if (v === undefined) {
      return MPEG_HEADER;
    }
    if (self._parsingId3v2) {
      self.id3v2.versionMinor = v[0];
      self.id3v2.flags = v[1];

      // calculate the length
      // from node-id3
      var offset = 2;
      var byte1 = v[offset],
          byte2 = v[offset + 1],
          byte3 = v[offset + 2],
          byte4 = v[offset + 3];
      self.id3v2.length =
             byte4 & 0x7f           |
             ((byte3 & 0x7f) << 7)  |
             ((byte2 & 0x7f) << 14) |
             ((byte1 & 0x7f) << 21);

      //self.id3v2.length = v.readInt32BE(2);
      //console.error(self.id3v2);

      self._parsingId3v2 = false;
      self._finishingId3v2 = true;
      self._id3v2_2 = v;
      return new strtok.BufferType(self.id3v2.length);
    }
    if (self._parsingId3v1) {
      self._parsingId3v1 = false;
      self.emit('id3v1', concat(self._id3v1_1, v));
      self._id3v1_1 = null;
      return MPEG_HEADER; // or DONE?
    }
    if (self._finishingId3v2) {
      //console.error('rest of id3v2');
      //console.error(v.length, v.toString());
      var b = concat(self._id3v2_1, self._id3v2_2, v)
      self._finishingId3v2 = false;
      self.emit('id3v2', b);
      return MPEG_HEADER;
    }
    // when we have a header buffer
    if (self.frameSize == -1) {
      var tag = v.toString('ascii', 0, 3);
      if (self.beginning && tag == 'ID3') {
        // beginning of id3v2 data
        self._parsingId3v2 = true;
        self.id3v2 = {};
        self.id3v2.versionMajor = v[3];
        self._id3v2_1 = v;
        // get the rest of the ID3v2 header
        return REST_OF_ID3V2_HEADER;
      }
      if (tag == 'TAG') {
        self._id3v1_1 = v;
        self._parsingId3v1 = true;
        return REST_OF_ID3V1;
      }
      self.beginning = false;
      self.frameHeader = parse(v);
      self.emit('header', v, self.frameHeader);
      self.frameSize = self.frameHeader.frameSize;
      return new strtok.BufferType(self.frameHeader.frameSize - MPEG_HEADER_LENGTH);
    }
    // when we have a frame buffer
    self.frame = v;
    self.emit('frame', self.frame);
    self.frameSize = -1; // reset
    return MPEG_HEADER;
  });
}
inherits(Parser, Stream)
module.exports = Parser


Parser.prototype.write = function write (b) {
  this.emit('data', b);
  return true;
}

Parser.prototype.end = function end (b) {
  if (b) this.emit('data', b);
  this.emit('end');
  return true;
}
