var Stream = require('stream')
  , inherits = require('util').inherits
  , parse = require('./parse').parseFrameHeader
  , strtok = require('strtok')
  , ID3V2_HEADER_LENGTH = 10
  , MPEG_HEADER_LENGTH = 4

var MPEG_HEADER = new strtok.BufferType(MPEG_HEADER_LENGTH);

// TODO: Parse out ID3v2 and ID3v1 tags
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
      console.error(self.id3v2);

      self._parsingId3v2 = false;
      self._finishingId3v2 = true;
      return new strtok.BufferType(self.id3v2.length);
    }
    if (self._finishingId3v2) {
      console.error('rest of id3v2');
      //console.error(v.length, v.toString());
      self._finishingId3v2 = false;
      return MPEG_HEADER;
    }
    // when we have a header buffer
    if (self.frameSize == -1) {
      if (self.beginning && v.toString('ascii', 0, 3) == 'ID3') {
        self._parsingId3v2 = true;
        self.id3v2 = {};
        self.id3v2.versionMajor = v[3];
        // get the rest of the ID3v2 header
        return new strtok.BufferType(6);
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
