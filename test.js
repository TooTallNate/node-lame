var fs = require('fs')
var lame = require('./')

var gfp = lame.malloc_gfp();
console.error('gfp wrapper:', gfp);
/*console.error('num_channels: %d', lame.lame_get_num_channels(gfp));
lame.lame_set_num_channels(gfp, 1);
console.error('num_channels: %d', lame.lame_get_num_channels(gfp));
lame.lame_set_num_channels(gfp, 2);*/
console.error('num_channels: %d', lame.lame_get_num_channels(gfp));

// get id3v2 data
var v2 = new Buffer(1500);
//console.error(v2);
var b0 = lame.lame_get_id3v2_tag(gfp, v2);
//console.error(v2);
console.error('id3v2 bytes:', b0);

console.error('framesize:', lame.lame_get_framesize(gfp));
lame.lame_init_params(gfp);
console.error('framesize:', lame.lame_get_framesize(gfp));

//lame.lame_print_config(gfp);
//lame.lame_print_internals(gfp);

//var pigs = fs.readFileSync('pigs.wav')
var pigs = fs.readFileSync('pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , num_samples = pigs.length / 4 // 4 is the sample size
  , mp3file = new Buffer(parseInt(1.25 * num_samples + 7200))
console.error('pigs.length:', pigs.length)
console.error('mp3file.length', mp3file.length);

console.error('num frames so far:', lame.lame_get_frameNum(gfp));
var b = lame.lame_encode_buffer_interleaved(gfp, pigs, num_samples, mp3file);
console.error('bytesWritten:', b);
console.error('num frames so far:', lame.lame_get_frameNum(gfp));

var b2 = lame.lame_encode_flush_nogap(gfp, mp3file.slice(b))
console.error('after flush:', b2);
console.error('num frames so far:', lame.lame_get_frameNum(gfp));

// ID3v1
var v1 = new Buffer(150);
//console.error(v1);
var b3 = lame.lame_get_id3v1_tag(gfp, v1);
//console.error(v1);
console.error('id3v1 bytes:', b3);

// Write mp3 data to 'pigs.mp3'
fs.writeFileSync('pigs.mp3', mp3file.slice(0, b + b2));

// call lame_close()
lame.lame_close(gfp);
