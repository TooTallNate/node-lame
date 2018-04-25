{
  'targets': [
    {
      'target_name': 'bindings',
      'sources': [
        'src/bindings.cc',
        'src/node_mpg123.cc'
      ],
      "include_dirs" : [
        '<!(node -e "require(\'nan\')")'
      ],
      'dependencies': [
        'deps/mpg123/mpg123.gyp:mpg123'
      ],
      'conditions':[
        ['OS=="win"', {
        'defines':[
          'NOMINMAX'
        ]
        }]
      ]
    }
  ]
}
