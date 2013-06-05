from SCons.Script import *


########################################################################
#
#  extract bitcode file from executable
#


__extract_action = '/home/rubio/ksvnrepo/projects/corvette/scripts/llvm-wrapper/extract-bc $SOURCES'


__extract_builder = Builder(
    action=__extract_action,
    suffix='.bc',
    source_scanner=CScanner,
    )


########################################################################


def generate(env):
    if 'Extract' in env['BUILDERS']:
        return

    env.AppendUnique(
        BUILDERS={'Extract': __extract_builder},
        )


def exists(env):
    return env.WhereIs('clang')
