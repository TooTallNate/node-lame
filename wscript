sjrcdir  = '.'
blddir  = 'build'
VERSION = '0.0.0'

def set_options(ctx):
  ctx.tool_options('compiler_cxx')

def configure(ctx):
  ctx.check_tool('compiler_cxx')
  ctx.check_tool('node_addon')

  ctx.check_cxx(lib="mp3lame")
  ctx.check_cxx(header_name='lame/lame.h', mandatory="True")

def build(ctx):
  t = ctx.new_task_gen('cxx', 'shlib', 'node_addon')
  t.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE"]
  t.target = 'nodelame'
  t.source = 'src/node-lame.cc'
  t.uselib = 'MP3LAME'
