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
  , endianness: 'little'  // Little-endian
});

// The decoder should have an MP3 file written to it
fs.createReadStream('/some/audio/file.mp3').pipe(decoder);

// Raw PCM data gets fed to stdout. play the stream with `ffplay` or somethin'
decoder.pipe(process.stdout);
```
