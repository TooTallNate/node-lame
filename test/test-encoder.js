var lame = require('../')
  , fs = require('fs')
  , filename = process.argv[2] || __dirname + '/../pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2';

var f = fs.createReadStream(filename)
  , encoder = lame.createEncoder()

f.on('data', function (b) {
  //console.error('file data. %d bytes.', b.length);
  var flushed = encoder.write(b, afterEncoderWrite);
  //console.error('flushed?', flushed);
});

function afterEncoderWrite () {
  console.error('afterEncoderWrite()');
}

encoder.pipe(process.stdout);
