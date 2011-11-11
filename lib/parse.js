// Adapted from:
// http://code.google.com/p/pcutmp3/source/browse/trunk/de/zebee/mpa/util/FrameHeader.java
//
// MPEG Header Info:
// http://www.mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm

var assert = require('assert')
  , UINT32_BE = require('strtok').UINT32_BE
  // some of there aren't used; oh well :)
  , ILLEGAL_MPEG_ID        = 1
  , MPEG1_ID               = 3
  , MPEG2_ID               = 2
  , MPEG25_ID              = 0
  , ILLEGAL_LAYER_ID       = 0
  , LAYER1_ID              = 3
  , LAYER2_ID              = 2
  , LAYER3_ID              = 1
  , ILLEGAL_SR             = 3
  , SR_32000HZ             = 2
  , SR_44100HZ             = 0
  , SR_48000HZ             = 1
  , MODE_MONO              = 3
  , MODE_DUAL              = 2
  , MODE_JOINT             = 1
  , MODE_STEREO            = 0

  , MPEG_NAME              = [ "MPEG2.5", null, "MPEG2", "MPEG1" ]
  , LAYER_NAME             = [ null, "Layer3", "Layer2", "Layer1" ]
  , MODE_NAME              = [ "Stereo", "J-Stereo", "Dual", "Mono" ]
  , SAMPLING_RATES         = [ 44100, 48000, 32000, 0 ]

  , BITRATE_MPEG1_LAYER1   = [ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 ]
  , BITRATE_MPEG1_LAYER2   = [ 0, 32, 48, 56, 64,  80, 96,   112, 128, 160, 192, 224, 256, 320, 384 ]
  , BITRATE_MPEG1_LAYER3   = [ 0, 32, 40, 48, 56,  64, 80,   96, 112, 128, 160, 192, 224, 256, 320 ]
  , BITRATE_MPEG2_LAYER1   = [ 0, 32, 48, 56, 64,  80, 96,   112, 128, 144, 160, 176, 192, 224, 256 ]
  , BITRATE_MPEG2_LAYER2A3 = [ 0, 8,  16, 24, 32,  40, 48,   56, 64, 80, 96, 112, 128, 144, 160 ]

  , BITRATE_MAP = [
      [ null, BITRATE_MPEG2_LAYER2A3, BITRATE_MPEG2_LAYER2A3, BITRATE_MPEG2_LAYER1 ] // MPEG2.5
    , null
    , [ null, BITRATE_MPEG2_LAYER2A3, BITRATE_MPEG2_LAYER2A3, BITRATE_MPEG2_LAYER1 ] // MPEG2
    , [ null, BITRATE_MPEG1_LAYER3, BITRATE_MPEG1_LAYER2, BITRATE_MPEG1_LAYER1 ]
  ]



exports.parseFrameHeader = function parseFrameHeader (b) {
  assert.ok(Buffer.isBuffer(b));
  assert.ok(b.length >= 4);

  var r = {};
  var header32 = UINT32_BE.get(b, 0);

  r.mpegID = (header32 >> 19) & 3;
  r.layerID = (header32 >> 17) & 3;
  r.crc16used = (header32 & 0x00010000) == 0;
  r.bitrateIndex = (header32 >> 12) & 0xF;
  r.samplingRateIndex = (header32 >> 10) & 3;
  r.samplingRateHz = SAMPLING_RATES[r.samplingRateIndex];
  r.padding = (header32 & 0x00000200) != 0;
  r.privateBitSet = (header32 & 0x00000100) != 0;
  r.mode = (header32 >> 6) & 3;
  r.modeExtension = (header32 >> 4) & 3;
  r.copyrighted = (header32 & 0x00000008) != 0;
  r.original = (header32 & 0x00000004) == 0; // bit set -> copy
  r.emphasis = header32 & 3;

  if (r.mpegID == MPEG2_ID)
    r.samplingRateHz >>= 1; // 16,22,48 kHz
  if (r.mpegID == MPEG25_ID)
    r.samplingRateHz >>= 2; // 8,11,24 kHz

  r.channels = (r.mode == MODE_MONO) ? 1 : 2;
  r.bitrateKBPS = BITRATE_MAP[r.mpegID][r.layerID][r.bitrateIndex];
  r.mpegName = MPEG_NAME[r.mpegID]
  r.layerName = LAYER_NAME[r.layerID];
  r.modeName = MODE_NAME[r.mode];

  if (r.layerID === LAYER1_ID) {
    // layer 1: always 384 samples/frame and 4byte-slots
    r.samplesPerFrame = 384;
    r.bytesPerSlot = 4;
    r.frameSizeRaw = (12 * (r.bitrateKBPS*1000) / (r.samplingRateHz*10) + (r.padding ? 1 : 0)) * 4;
  } else {
    // layer 2: always 1152 samples/frame
    // layer 3: MPEG1: 1152 samples/frame, MPEG2/2.5: 576 samples/frame
    r.samplesPerFrame = ((r.mpegID === MPEG1_ID) || (r.layerID === LAYER2_ID)) ? 1152 : 576;
    r.bytesPerSlot = 1;
    r.frameSizeRaw = 144 * (r.bitrateKBPS*1000) / r.samplingRateHz + (r.padding ? 1 : 0);
  }
  // Make the frameSize be the proper floor'd byte length
  r.frameSize = ~~r.frameSizeRaw;

  if (!r.frameSize)
    throw new Error('bad size: ' + r.frameSize);

  return r;
}
