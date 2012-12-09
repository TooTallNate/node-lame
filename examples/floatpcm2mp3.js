var fs = require('fs');
var lame = require('../');
var path = require('path');

var encoder = new lame.Encoder({
	bitDepth: 32, 
	float: true
});

encoder.on('end', function() {

	console.log('Encodeing ended!');
});

var pcmInput = fs.createReadStream(path.resolve(__dirname, 'sample.float.pcm'));
var mp3Out = fs.createWriteStream(path.resolve(__dirname, 'sample_pcm.mp3'));

pcmInput.pipe(encoder);
encoder.pipe(mp3Out);

