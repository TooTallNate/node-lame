var Stream = require('stream')
  , inherits = require('util').inherits
  , parse = require('./parse').parseFrameHeader
  , strtok = require('strtok')
  , MPEG_HEADER_LENGTH = 4

var MPEG_HEADER = new strtok.BufferType(MPEG_HEADER_LENGTH);

// TODO: Parse out ID3v2 and ID3v1 tags
function Parser () {
  Stream.call(this);
  this.readable = this.writable = true;
  this.frameSize = -1;
  var self = this;
  strtok.parse(this, function (v, cb) {
    // start things off
    if (v === undefined) {
      return MPEG_HEADER;
    }
    // when we have a header buffer
    if (self.frameSize == -1) {
      console.error(v)
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
