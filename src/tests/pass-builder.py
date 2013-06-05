from SCons.Script import *


########################################################################
#
#  generate a builder for weighted pushdown systems
#


def PASSBuilder(env, engine, action, suffix='.out', target_scanner=None, **kwargs):

    def pass_generator(target, source, env, for_signature):
        actions = [action]
        return actions

    def pass_target_scanner(node, env, path):
        deps = [env[engine]]
        if target_scanner:
            deps += target_scanner(node, env, path)
        return deps

    return Builder(generator=pass_generator, target_scanner=Scanner(pass_target_scanner), suffix=suffix, **kwargs)


########################################################################


def generate(env):
    if hasattr(env, 'PASSBuilder'):
        return

    env.AddMethod(PASSBuilder)


def exists(env):
    return True
