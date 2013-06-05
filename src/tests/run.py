from SCons.Script import *


########################################################################
#
#  running a bitcode file
#


__run_action = 'lli $SOURCES $TARGETS'


__run_builder = Builder(
    action=__run_action,
    src_suffix=['.bc'],
    source_scanner=CScanner,
    )


########################################################################


def generate(env):
    if 'Run' in env['BUILDERS']:
        return

    env.AppendUnique(
        BUILDERS={'Run': __run_builder},
        )


def exists(env):
    return env.WhereIs('lli')
