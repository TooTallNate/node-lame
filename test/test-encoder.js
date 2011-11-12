var lame = require('./')
  , fs = require('fs')
  , Parser = require('./lib/parser')

var f = fs.createReadStream('pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , encoder = lame.createEncoder()

f.pipe(encoder);

var parser = new Parser();
encoder.pipe(parser);

parser.on('header', function (b, meta) {
  console.error('header:', meta);
  process.stdout.write(b);
}).on('frame', function (b) {
  console.error('frame');
  process.stdout.write(b);
});
