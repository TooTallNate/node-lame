
/**
 * Proof of concept, basically, of the low-level bindings.
 */

var fs = require('fs')
  , assert = require('assert')
  , lame = require('../lib/bindings')
  , parse = require('../lib/parse')

var gfp = lame.lame_init();
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

//console.error('rate result:', lame.lame_set_out_samplerate(gfp, 48000))
lame.lame_init_params(gfp);
var s = lame.lame_get_framesize(gfp);
console.error('framesize:', lame.lame_get_framesize(gfp));
console.error('VBR mode: %d', lame.lame_get_VBR(gfp));

//var pigs = fs.readFileSync('pigs.wav')
var pigs = fs.readFileSync(__dirname+'/../pigs.f.s16le.acodec.pcm_s16le.ar.44100.ac.2')
  , num_samples = pigs.length / 4 // 4 is the sample size
  , mp3file = new Buffer(parseInt(1.25 * num_samples + 7200))
console.error('pigs.length:', pigs.length)
console.error('mp3file.length', mp3file.length);
console.error('frame size:', s)

console.error('num frames so far:', lame.lame_get_frameNum(gfp));

lame.lame_encode_buffer_interleaved(gfp, pigs, num_samples, mp3file, function (err, b) {
  console.error('after encode:',err, b);


console.error('bytesWritten:', b);
console.error('num frames so far:', lame.lame_get_frameNum(gfp));

var b2 = lame.lame_encode_flush_nogap(gfp, mp3file.slice(b))
console.error('after flush:', b2);
console.error('num frames so far:', lame.lame_get_frameNum(gfp));

var i = 0
  , cur = mp3file.slice(0, b + b2)
do {
  if (!cur.length) break;
  if (cur.length < 100000) {
    mp3file = cur;
    break;
  }
  console.error(0, 'parsing', cur.slice(0, 4));
  var par = parse.parseFrameHeader(cur)
  console.error(par);
  cur = cur.slice(par.frameSize)
  if (cur[0] === 0) break;
  i++
} while (true)
console.error(cur.length)

// ID3v1
var v1 = new Buffer(150);
//console.error(v1);
var b3 = lame.lame_get_id3v1_tag(gfp, v1);
//console.error(0, v1.toString())
console.error('id3v1 bytes:', b3);

// Write mp3 data to 'pigs.mp3'
//fs.writeFileSync('pigs.mp3', mp3file.slice(0, b + b2));
fs.writeFileSync('pigs.mp3', mp3file);

// call lame_close()
lame.lame_close(gfp);


});
