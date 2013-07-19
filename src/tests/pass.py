from SCons.Script import *


########################################################################
#
#  apply transformations to LLVM bitcode file
#


def plugin_ld_library_path(target, source, env, for_signature):
    compiler = env.WhereIs('$CXX')
    bindir = File(compiler).dir
    if bindir.path != '/usr/bin':
        libdir = Dir('lib', bindir.dir)
        envar = 'LD_LIBRARY_PATH=%s' % libdir
        return envar


def generate(env):
    builder_name = 'LlvmPass'
    if builder_name in env['BUILDERS']:
        return

    env.Tool('pass-builder', toolpath=['#src/tests'])

    builder = env.PASSBuilder(
        engine='PASS_PLUGIN',
        action='$__VALGRIND $__PLUGIN_LD_LIBRARY_PATH opt -load $PASS_PLUGIN -variables -adjust-operators --die --time-passes -include=$INCLUDE_FILE -exclude=$EXCLUDE_FILE -json-config=$CONFIG_FILE -output=$TARGET $SOURCES > $TARGET',
        src_suffix=['.bc', '.ll'],
        src_builder='Extract',
        )

    env.AppendUnique(
        BUILDERS={builder_name: builder},
        PASS_PLUGIN=env.File('#src/Passes$SHLIBSUFFIX'),
        __PLUGIN_LD_LIBRARY_PATH=plugin_ld_library_path,
        )


def exists(env):
    return True
