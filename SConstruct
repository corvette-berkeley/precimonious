from SCons.Script import *
from os import access, environ, X_OK
import platform

#CacheDir('.scons-cache')
Decider('MD5-timestamp')
SetOption('implicit_cache', True)
SourceCode('.', None)


########################################################################
#
#  various command-line options
#

def pathIsExecutable(key, val, env):
    found = env.WhereIs(val)
    if found: val = found
    PathVariable.PathIsFile(key, val, env)
    if not access(val, X_OK):
        raise SCons.Errors.UserError('Path for option %s is not executable: %s' % (key, val))

opts = Variables(['.scons-options'], ARGUMENTS)
opts.Add(BoolVariable('DEBUG', 'Compile with extra information for debugging', False))
opts.Add(BoolVariable('OPTIMIZE', 'Compile with optimization', False))
opts.Add(BoolVariable('NATIVECAML', 'Use the native-code OCaml compiler', True))
opts.Add(BoolVariable('PROFILE', 'Turn on performance profiling', False))
opts.Add(BoolVariable('VALGRIND', "Run tests under Valgrinds's memory checker", False))
opts.Add(PathVariable('LLVM_CONFIG', 'Path to llvm-config executable', WhereIs('llvm-config'), pathIsExecutable))




if platform.system() == 'Darwin':
    Is64 = False
else:
    Is64 = platform.architecture()[0] == '64bit'

env = Environment(
		CCFLAGS = ['-Isrc/lib'],
    options=opts,
    Is64=Is64,
    )

env.PrependENVPath('PATH', [
        '/home/rubio/build-llvm/Release/bin',
        '/usr/local/bin',
        '/opt/local/bin',
        '/unsup/ocaml/bin',
        '/s/texlive-2008/bin',
        '/s/std/bin',
        ])


########################################################################
#
#  basic LaTeX document rendering
#

env.AppendUnique(
        COMMONLATEXFLAGS=['-file-line-error', '-interaction=batchmode'],
        LATEXFLAGS='$COMMONLATEXFLAGS',
        PDFLATEXFLAGS='$COMMONLATEXFLAGS',
        BIBTEXFLAGS='-terse',
        )


########################################################################
#
#  shared compiliation flags
#

flags = [
    '-Wall',
    '-Wformat=2',
    '-Wextra',
    '-Werror',
    '${("", "-g")[DEBUG]}',
    '${("", "-O")[OPTIMIZE]}',
    ]

env.AppendUnique(
    CCFLAGS=flags,
    LINKFLAGS=flags,
    )


########################################################################
#
#  subsidiary scons scripts
#

SConscript(
    dirs=[
        # our stuff
	'src',
        ],
    exports='env',
    )
