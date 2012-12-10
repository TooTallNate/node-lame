var fs = require('fs');
var lame = require('../');
var path = require('path');

fs.createReadStream(path.resolve(__dirname, 'sample.float.pcm'))
  .pipe(new lame.Encoder({ bitDepth: 32, float: true }))
  .pipe(fs.createWriteStream(path.resolve(__dirname, 'sample_pcm.mp3')))
  .on('close', function () {
    console.error('done!')
  });
