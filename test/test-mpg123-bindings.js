var bindings = require('../').bindings;

bindings.mpg123_init();
var mh = bindings.mpg123_new();

if (typeof mh == 'number') {
  // error
}
console.error(mh);

console.error(bindings.mpg123_current_decoder(mh));

var ret;

ret = bindings.mpg123_open_feed(mh);

console.error(ret, ret == bindings.MPG123_OK);

var OUTBUFF = 32768;

process.stdin.resume();
process.stdin.on('data', function (d) {

  var len = d.length;
  var out = new Buffer(OUTBUFF);

  console.error('decoding')
  ret = bindings.mpg123_decode(mh, d, len, out, 0, OUTBUFF);
  console.error(ret);
  if (ret.ret == bindings.MPG123_NEW_FORMAT) {
    console.error('NEW_FORMAT');
  }
  if (ret.size) {
    process.stdout.write(out.slice(0, ret.size));
  }
  var count = 0;
  while (ret.ret != bindings.MPG123_ERR && ret.ret != bindings.MPG123_NEED_MORE) {
    out = new Buffer(OUTBUFF);
    console.error('decoding while ', count++)

    ret = bindings.mpg123_decode(mh, null, 0, out, 0, OUTBUFF);
    console.error(ret)
    if (ret.size) {
      process.stdout.write(out.slice(0, ret.size));
    }
  }
  if (ret.ret == bindings.MPG123_ERR) {
    console.error('ERROR!!!!!')
  }
});

process.stdin.on('end', function () {
  process.stdout.end();
  process.exit(0);
});
