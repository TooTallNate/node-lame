var fs = require('fs')
  , Parser = require('./lib/parser')

var f = fs.createReadStream('pigs.good.mp3')
var parser = new Parser();
f.pipe(parser);

parser.on('header', function (b, meta) {
  //console.error('header', meta);
  process.stdout.write(b);
}).on('frame', function (b) {
  //console.error('frame');
  process.stdout.write(b);
}).on('id3v2', function (b) {
  console.error('id3v2');
  console.error(b.length, b.toString('binary', 0, 100));
}).on('id3v1', function (b) {
  console.error('id3v1');
  console.error(b.length, b.toString());
});
