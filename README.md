node-lame
=========
### NodeJS native bindings to libmp3lame

For all your async streaming MP3 encoding/decoding needs, there's `node-lame`!
This module hooks into libmp3lame, the library that the `lame` command uses, to
provide an `Encoder` and `Decoder` class to NodeJS.


Installation
------------

Install with `npm`:

``` bash
$ npm install lame
```


Example
-------

Here's an example of using `node-lame` to decode an MP3 file to raw PCM data and
pipe that to `process.stdout`:

``` javascript
var fs = require('fs')
  , lame = require('lame')

// Create the Decoder instance
var decoder = lame.createDecoder({
    channels: 2           // 2 channels (left and right)
  , signed: true          // Signed data values
  , sampleSize: 16        // 16-bit samples
  , sampleRate: 44100     // 44,100 Hz sample rate
  , endianness: 'little'  // Little-endian samples
});

// The decoder should have an MP3 file written to it
fs.createReadStream('/some/audio/file.mp3').pipe(decoder);

// Raw PCM data gets fed to stdout. play the stream with `ffplay` or somethin'
decoder.pipe(process.stdout);
```


API
---

### Decoder class

The `Decoder` class is a `Stream` subclass that accepts MP3 data written to it,
and emits raw PCM as `data` events.

### Encoder class

The `Encoder` class is a `Stream` subclass that accepts raw PCM data written to
it, and emits MP3 data as `data` events.

### Parser class

The `Parser` class is an `EventEmitter` subclass that accepts MP3 data being
written to it, and emits events as the different structures of the MP3 file are
encountered:

  * `id3v2` event - fired one time when the ID3v2 data at the beginning of the MP3 file has been parsed. A single `Buffer` argument is passed to the callbacks.
  * `frame` events - fired multiple times as each complete MP3 frame gets written. A single `Buffer` argument is passed that is a complete MP3 frame.
  * `id3v1` event - fired one time when the ID3v1 data at the end of the MP3 file has been parsed. A single `Buffer` argument is passed that is the ID3 info.
  * `end` event - fired one time when then end of the MP3 file is encountered.
