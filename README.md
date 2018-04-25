node-addon-mpg123
=========

### NodeJS addon & native bindings to libmpg123
[![Build Status](https://travis-ci.org/mdluo/node-addon-mpg123.svg?branch=master)](https://travis-ci.org/mdluo/node-addon-mpg123)

`node-addon-mpg123` is based on [TooTallNate/node-lame](https://github.com/TooTallNate/node-lame) and removed `lame` related code to keep it simple. *And extended the `Decoder` to support decoding format  options.*

For all your async streaming MP3 decoding needs, there's `node-addon-mpg123`!
This module hooks into libmpg123, the library that the `mpg123` command uses, to
provide `Decoder` streams to NodeJS.


Installation
------------

`node-addon-mpg123` comes bundled with its own copy of `libmpg123`, so
there's no need to have them installed on your system.

Simply compile and install `node-addon-mpg123` using `npm`:

``` bash
$ npm install node-addon-mpg123
```

Example
-------

Here's an example of using `node-addon-mpg123` to decode an MP3 file coming from
`process.stdin` to some raw PCM data that gets piped to `process.stdout`:

``` javascript
const mpg123 = require('node-addon-mpg123');

// create the Decoder instance
const decoder = new mpg123.Decoder();

// MP3 data from stdin gets piped into the decoder
process.stdin.pipe(decoder);

// the raw PCM data gets piped to stdout
decoder.pipe(process.stdout);
```

See `test/decoder.js` for some more example code.

API
---

### Decoder class

The `Decoder` class is a `Stream` subclass that accepts MP3 data written to it,
and outputs raw PCM data. It also emits a `"format"` event when the format of
the MP3 file is determined (usually right at the beginning). You can specify
the output PCM data format when creating the decoder instance.

```javascript
const mpg123 = require('node-addon-mpg123');
const decoder = new mpg123.Decoder({
  sampleRate: 44100,        // [8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000]
  channels: mpg123.STEREO,  // [mpg123.MONO, mpg123.STEREO, mpg123.MONO | mpg123.STEREO]
  signed: false,            // [true, false]
  float: true,              // [true, false]
  bitDepth: 32,             // [8, 16, 24, 32]
});
```

⚠️ The `channels` option is different from the `TooTallNate/node-lame` encoder.

See more about mpg123 encoding formats: https://github.com/mdluo/node-addon-mpg123/blob/master/deps/mpg123/src/libmpg123/mpg123.h.in#L348-L395
