
var fs = require('fs');
var path = require('path');
var lame = require('../');
var assert = require('assert');
var fixtures = path.resolve(__dirname, 'fixtures');

describe('Decoder', function () {

  describe('pipershut_lo.mp3', function ()  {
    var filename = path.resolve(fixtures, 'pipershut_lo.mp3');

    it('should emit a "format" event', function (done) {
      var file = fs.createReadStream(filename);
      var decoder = new lame.Decoder();
      decoder.on('format', function (format) {
        assert(format);
        done();
      });
      file.pipe(decoder);
    });

  });

});
