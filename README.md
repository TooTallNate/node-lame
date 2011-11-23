node-lame
=========
### NodeJS native bindings to libmp3lame & libmpg123

For all your async streaming MP3 encoding/decoding needs, there's `node-lame`!
This module hooks into libmp3lame, the library that the `lame` command uses, to
provide an `Encoder` and `Decoder` class to NodeJS.


Installation
------------

First you must have the lame dev files installed (`lame.h`) and the mpg123 dev
files installed (`mpg123.h`).

On OS X with Homebrew, this is as simple as:

``` bash
$ brew install lame mpg123
```

Or on Ubuntu, try this:

``` bash
$ apt-get install libmp3lame-dev libmpg123-dev
```

Of course you can install those packages from source yourself if you prefer.

Now that the native libraries are installed, we can compile and install
`node-lame` using `npm`:

``` bash
$ npm install lame
```


Example
-------

Here's an example of using `node-lame` to encode some raw PCM data coming from
`process.stdin` to an MP3 file that gets piped to `process.stdout`:

``` javascript
var fs = require('fs')
  , lame = require('lame')

// Create the Encoder instance
var encoder = lame.createEncoder({
    channels: 2           // 2 channels (left and right)
  , signed: true          // Signed data values
  , sampleSize: 16        // 16-bit samples
  , sampleRate: 44100     // 44,100 Hz sample rate
  , endianness: 'little'  // Little-endian samples
});

// Raw PCM data from stdin gets piped into the encoder.
process.stdin.pipe(encoder);

// The generated MP3 file gets piped to stdout.
encoder.pipe(process.stdout);
```


API
---

### Decoder class

#### Note: Decoder class is still a work-in-progress
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
  * `header` events - fired multiple times as each MP3 frame header gets written. A single `Buffer` argument is passed that is the header.
  * `frame` events - fired multiple times as each complete MP3 frame gets written. A single `Buffer` argument is passed that is a complete MP3 frame.
  * `id3v1` event - fired one time when the ID3v1 data at the end of the MP3 file has been parsed. A single `Buffer` argument is passed that is the ID3 info.
  * `end` event - fired one time when then end of the MP3 file is encountered.
