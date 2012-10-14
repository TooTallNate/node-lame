{
  'targets': [
    {
      'target_name': 'bindings',
      'sources': [
        'src/bindings.cc',
        'src/node_lame.cc'
      ],
      'dependencies': [
        'deps/lame/libmp3lame.gyp:mp3lame',
        'deps/mpg123/mpg123.gyp:mpg123'
      ]
    }
  ]
}
