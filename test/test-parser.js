var id3 = require('id3')
  , Parser = require('../lib/parser')

// Read an MPEG file from stdin
var f = process.stdin;
f.resume();

// Create a Parser instance
var parser = new Parser();

// pipe stdin to the parser
f.pipe(parser);

// emits 'header' events on each MPEG header (start of frame)
parser.on('header', function (b, meta) {
  //console.error('header', meta);
  process.stdout.write(b);
});

// emits 'frame' event for the actual data from each frame
parser.on('frame', function (b) {
  //console.error('frame');
  process.stdout.write(b);
});

// emits 'id3v2' when the ID3v2 section at the beginning has been parsed
parser.on('id3v2', function (b) {
  var tags = new id3(b);
  console.error(tags.getTags());
  console.error('id3v2. %d bytes', b.length);
  process.stdout.write(b);
});

// emits 'id3v1' event when the ID3v1 section at the end has been parsed
parser.on('id3v1', function (b) {
  var tags = new id3(b);
  console.error(tags.getTags());
  console.error('id3v1. %d bytes', b.length);
  process.stdout.write(b);
});
