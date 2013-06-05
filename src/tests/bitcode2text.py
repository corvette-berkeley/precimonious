from SCons.Action import Action
from SCons.Builder import Builder


########################################################################
#
#  generate a sorted file
#


def __bitcode2text_emitter(target, source, env):
    #source.append(env['SortFile'])
    return target, source


__bitcode2text_action = Action([[
            '$Bitcode2Text.abspath', '$SOURCES', '>$TARGET'
            ]])


__bitcode2text_builder = Builder(
    emitter=__bitcode2text_emitter,
    action=__bitcode2text_action,
    )




########################################################################


def generate(env):

    env.AppendUnique(
        Bitcode2Text=env.File('compare.sh'),
        BUILDERS={'Bitcode2Text': __bitcode2text_builder,
           },
        )


def exists(env):
    return True
