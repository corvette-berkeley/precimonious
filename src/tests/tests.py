from ConfigParser import RawConfigParser
from itertools import imap
from SCons.Script import *
from sets import Set

def needsTestSuite(target):
    """check whether this target needs test suite details"""

    if str(target) == 'test':
        return True

    if str(target).startswith('test-'):
        return True

    if isinstance(target, str):
        target = Entry('#' + target)

    return False


def Tests(env):
    """return sequence of all enabled tests, ordered by test number"""
    return sorted(env['tests'], key=lambda item: int(item.name[4:]))


def generate(env):
    if 'tests' in env:
        return

    env['tests'] = set()
    env.AddMethod(Tests)

    # skip test suite details unless something actually needs them
    if not any(imap(needsTestSuite, BUILD_TARGETS)):
        return

    # simplified version uses sets instead of dictionaries
    for subdir in env.Glob('#src/tests/test*'):
        if not subdir.isdir():
            continue

        env['tests'].add(subdir)


def exists(env):
    return True
