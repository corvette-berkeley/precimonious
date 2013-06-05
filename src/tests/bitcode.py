from SCons.Script import *


########################################################################
#
#  generate LLVM bitcode from C/C++ source using the Clang front end
#


__bitcode_action = 'clang -c -emit-llvm -o $TARGET $SOURCES'


__bitcode_builder = Builder(
    action=__bitcode_action,
    src_suffix=['.c', '.cpp'],
    suffix='.bc',
    source_scanner=CScanner,
    )


########################################################################


def generate(env):
    if 'Bitcode' in env['BUILDERS']:
        return

    env.AppendUnique(
        BUILDERS={'Bitcode': __bitcode_builder},
        )


def exists(env):
    return env.WhereIs('clang')
