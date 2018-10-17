
var fs = require('fs');
var path = require('path');
var lame = require('../');
var assert = require('assert');
var fixtures = path.resolve(__dirname, 'fixtures');
var outputName = path.resolve(__dirname, 'output.wav');

describe('Decoder', function () {

  describe('pipershut_lo.mp3', function ()  {
    var title = 'The Burning of the Piper\'s Hut';
    var artist = 'Tony Cuffe & Billy Jackson';
    var album = 'Sae Will We Yet';
    var year = '2003';
    var comment = 'sample of full track';
    var trackNumber = 8;
    var genre = 88;
    var filename = path.resolve(fixtures, 'pipershut_lo.mp3');

    it('should emit "readable" events', function (done) {
      var file = fs.createReadStream(filename);
      var output = fs.createWriteStream(outputName);
      var count = 0;
      var decoder = new lame.Decoder();
      decoder.on('readable', function () {
        count++;
      });
      decoder.on('finish', function () {
        assert(count > 0);
        fs.unlinkSync(outputName);
        done();
      });
      file.pipe(decoder).pipe(output);
    });

    it('should emit a single "format" event', function (done) {
      var file = fs.createReadStream(filename);
      var decoder = new lame.Decoder();
      decoder.on('format', function (format) {
        assert(format);
        done();
      });
      file.pipe(decoder);
    });

    it('should set correct output encoding format', function (done) {
      var file = fs.createReadStream(filename);
      var decoder = new lame.Decoder({
        sampleRate: 44100,
        channels: 1,
        signed: false,
        float: true,
        bitDepth: 32,
      });
      decoder.on('format', function (format) {
        assert(format);
        assert.equal(0x200, format.raw_encoding);
        assert.equal(44100, format.sampleRate);
        assert.equal(1, format.channels);
        assert.equal(false, format.signed);
        assert.equal(true, format.float);
        assert.equal(32, format.bitDepth);
        done();
      });
      file.pipe(decoder);
    });

    it('should throw error on unsupported sampleRate', function (done) {
      var file = fs.createReadStream(filename);
      assert.throws(function () {
        var decoder = new lame.Decoder({
          sampleRate: 44200,
        });
        file.pipe(decoder);
      }, /unsupported output format/)
      done();
    });

    it('should throw error on unsupported channels', function (done) {
      var file = fs.createReadStream(filename);
      assert.throws(function () {
        var decoder = new lame.Decoder({
          channels: 4,
        });
        file.pipe(decoder);
      }, /unsupported output format/)
      done();
    });

    it('should throw error on unsupported bitDepth', function (done) {
      var file = fs.createReadStream(filename);
      assert.throws(function () {
        var decoder = new lame.Decoder({
          bitDepth: 30,
        });
        file.pipe(decoder);
      }, /unsupported output format/)
      done();
    });

    it('should emit a single "finish" event', function (done) {
      var file = fs.createReadStream(filename);
      var output = fs.createWriteStream(outputName);
      var decoder = new lame.Decoder();
      decoder.on('finish', done);
      file.pipe(decoder).pipe(output);
    });

    it('should emit a single "end" event', function (done) {
      var file = fs.createReadStream(filename);
      var output = fs.createWriteStream(outputName);
      var decoder = new lame.Decoder();
      decoder.on('end', function () {
        fs.unlinkSync(outputName);
        done();
      });
      file.pipe(decoder).pipe(output);
    });

    it('should emit a single "id3v1" event', function (done) {
      var file = fs.createReadStream(filename);
      var decoder = new lame.Decoder();
      decoder.on('id3v1', function (id3) {
        assert.equal(title, id3.title);
        assert.equal(artist, id3.artist);
        assert.equal(album, id3.album);
        assert.equal(year, id3.year);
        assert.equal(comment, id3.comment);
        assert.equal(trackNumber, id3.trackNumber);
        assert.equal(genre, id3.genre);
        done();
      });
      file.pipe(decoder);

      // enable "flow"
      decoder.resume();
    });

    it('should emit a single "id3v2" event', function (done) {
      var file = fs.createReadStream(filename);
      var decoder = new lame.Decoder();
      decoder.on('id3v2', function (id3) {
        assert.equal(title, id3.title);
        assert.equal(artist, id3.artist);
        assert.equal(album, id3.album);
        assert.equal(year, id3.year);
        assert.equal(comment, id3.comment);
        done();
      });
      file.pipe(decoder);

      // enable "flow"
      decoder.resume();
    });

  });

});
