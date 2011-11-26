var Config = require('../lib/config').Config
  , assert = require('assert')

var c = new Config();

// bitrate
console.error(c.bitrate);

c._initParams();

// bitrate
console.error(c.bitrate);

c.bitrate = 320;
console.error(c.bitrate);
