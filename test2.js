var lame = require('./')
  , fs = require('fs')

var f = fs.createReadStream('pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , encoder = lame.createEncoder();

f.pipe(encoder);
encoder.pipe(process.stdout);
