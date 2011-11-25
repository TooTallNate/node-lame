var lame = require('../')
  , fs = require('fs')
  , filename = process.argv[2] || __dirname + '/../pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2';

var f = fs.createReadStream(filename)
  , encoder = lame.createEncoder();

f.pipe(encoder);
encoder.pipe(process.stdout);

var p = encoder.pause;
encoder.pause = function () {
  console.error('pause');
  p.apply(this, arguments);
}

encoder.on('drain', function () {
  console.error('drain');
});
