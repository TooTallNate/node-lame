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

  ctx.check_cxx(lib="mpg123")
  ctx.check_cxx(header_name='mpg123.h', mandatory="True")

def build(ctx):
  t = ctx.new_task_gen('cxx', 'shlib', 'node_addon')
  t.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE"]
  t.target = 'bindings'
  t.source = 'src/bindings.cc src/node_lame.cc src/node_mpg123.cc'
  t.uselib = 'MP3LAME MPG123'
