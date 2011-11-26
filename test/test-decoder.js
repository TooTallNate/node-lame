var lame = require('../')
  , fs = require('fs')
  , assert = require('assert')

fs.createReadStream(process.argv[2] || __dirname + '/../pigs.mp3')
  .pipe(lame.createDecoder())
  .pipe(process.stdout);
