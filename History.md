
1.1.1 / 2014-07-28
==================

 * travis: don't test node v0.6, v0.9
 * travis: test node v0.10, v0.11
 * package: update all dependency versions
 * README: use svg for Travis badge
 * examples: fix require() call in mp3player.js example
 * encoder: added support for stereo modes (#29, @benjamine)
 * decoder: never output NUL bytes in the ID3 strings
 * decoder, encoder: always use "readable-stream"

1.1.0 / 2014-04-28
==================

 * encoder: added support for mono audio (`channels: 1`, #26)
 * encoder: better `pcm_type` handling
