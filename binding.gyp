{
  'targets': [
    {
      'target_name': 'bindings',
      'sources': [
        'src/bindings.cc',
        'src/node_lame.cc',
        'src/node_mpg123.cc'
      ],
      'dependencies': [
        'deps/lame/libmp3lame.gyp:mp3lame',
        'deps/mpg123/mpg123.gyp:mpg123'
      ],
      'conditions': [
        ['OS=="mac" and target_arch=="ia32"', {
          # fixes: ld: illegal text-relocation to _INT123_costab_mmxsse in
          # Release/libmpg123.a(tabinit_mmx.o) from _INT123_dct64_MMX in
          # Release/libmpg123.a(dct64_mmx.o) for architecture i386
          'libraries': [ '-read_only_relocs suppress' ]
        }]
      ]
    }
  ]
}
