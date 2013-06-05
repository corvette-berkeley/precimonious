from SCons.Script import *

import filecmp


########################################################################
#
#  comparison with reference output
#


def __compare_action_exec(target, source, env):
    __pychecker__ = 'no-argsused'
    [_source] = source
    linestring = open(str(_source), 'r').read()
    if 'true' in linestring:
        same = True
    else:
        same = False
    return ('file contents mismatch', False)[same]


def __compare_action_show(target, source, env):
    __pychecker__ = 'no-argsused'
    [_source] = source 
    return 'Checking result value in file "%s"' % str(_source)


__compare_action = Action(__compare_action_exec, __compare_action_show)


__compare_action = [__compare_action, Touch('$TARGET')]


__compare_builder = Builder(
    action=__compare_action,
    suffix='.passed',
    )


########################################################################


def generate(env):
    env.AppendUnique(
        BUILDERS={'Compare': __compare_builder},
        )


def exists(env):
    return True
