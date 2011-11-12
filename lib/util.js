
exports.buffer_concat = function buffer_concat (bufs) {
  var buffer, length = 0, index = 0;

  if (!Array.isArray(bufs)) {
    bufs = Array.prototype.slice.call(arguments);
  }
  for (var i=0, l=bufs.length; i<l; i++) {
    buffer = bufs[i];
    length += buffer.length;
  }
  buffer = new Buffer(length);

  bufs.forEach(function (buf, i) {
    buf = bufs[i];
    buf.copy(buffer, index, 0, buf.length);
    index += buf.length;
  });

  return buffer;
}
