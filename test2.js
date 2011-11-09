var lame = require('./')
  , fs = require('fs')
  , BlockStream = require('block-stream')

var f = fs.createReadStream('pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , encoder = lame.createEncoder()
  , s = encoder.framesize
  , bs = new BlockStream(s, { nopad: true })

console.error('framesize:', s)

f.pipe(encoder);
encoder.pipe(bs);

encoder.once('data', function (b) {
  console.error('framesize:', encoder.framesize)
  console.error('framesize/2:', encoder.framesize/2)
  console.error(b.length);
  var num = ~~(b.length / s)
  console.error('count:', num);
//console.error(b)
  /*for (var i=0; i<num; i++) {
    console.error(0, b.slice(i*s, (i*s)+4))
  }*/
  for (var l=0; l<b.length; l++) {
    if (b[l] == 255) {
      console.error(l, b.slice(l, l+4))
    }
  }
});
bs.pipe(process.stdout);

/*bs.on('data', function (frame) {

  console.error('got frame')
  console.error(frame.length)
  console.error(0, frame.slice(0, 4))
});*/
