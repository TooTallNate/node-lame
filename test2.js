var lame = require('./')
  , fs = require('fs')
  , Parser = require('./lib/parser')
  , BlockStream = require('block-stream')

var f = fs.createReadStream('pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , encoder = lame.createEncoder()
  //, bs = new BlockStream(s, { nopad: true })

f.pipe(encoder);
//encoder.pipe(bs);

var parser = new Parser();
encoder.pipe(parser);

parser.on('header', function (b, meta) {
  console.error('header:', meta);
  process.stdout.write(b);
}).on('frame', function (b) {
  console.error('frame');
  process.stdout.write(b);
});



/*bs.on('data', function (frame) {

  console.error('got frame')
  console.error(frame.length)
  console.error(0, frame.slice(0, 4))
});*/
