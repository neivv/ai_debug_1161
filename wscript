#!/usr/bin/env python
# coding: utf-8

import os

def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--debug', action='store_true', help='"Debug" build', dest='debug')
    opt.add_option('--nodebug', action='store_true', help='Release build (Default)', dest='nodebug')
    opt.add_option('--optimize', action='store_true', help='Optimize always', dest='optimize')
    opt.add_option('--static', action='store_true', help='Enable static linkage (Default)', dest='static')
    opt.add_option('--nostatic', action='store_true', help='Disable static linkage', dest='nostatic')
    opt.add_option('--console', action='store_true', help='Console support (Requires freetype2)', dest='console')
    opt.add_option('--noconsole', action='store_true', help='Disable console (default)', dest='noconsole')
    opt.add_option('--freetype-include', action='store', help='Freetype include directory', dest='ft_inc')
    opt.add_option('--freetype-lib', action='store', help='Freetype library directory', dest='ft_lib')
    opt.add_option('--freetype-link', action='store', help='Freetype link flags', dest='ft_link')

def configure(conf):
    import waflib.Tools.compiler_cxx as compiler_cxx
    import waflib.Errors

    conf.env.MSVC_TARGETS = 'x86'

    compiler_cxx.configure(conf)

    if conf.options.ft_inc and conf.options.ft_lib:
        conf.env.ft_inc = conf.options.ft_inc
        conf.env.ft_lib = conf.options.ft_lib
        conf.env.ft_link = conf.options.ft_link
    else:
        try:
            conf.check_cfg(package='freetype2', args='--cflags --libs', uselib_store='freetype')
            conf.env.freetype_pkg = True
        except waflib.Errors.ConfigurationError:
            conf.fatal('Freetype not found, please pass --freetype-include and --freetype-lib manually')

    conf.env.debug = conf.options.debug and not conf.options.nodebug
    conf.env.optimize = conf.options.optimize
    conf.env.static = conf.options.static or not conf.options.nostatic

    conf.env.cshlib_PATTERN = conf.env.cxxshlib_PATTERN = '%s.qdp'

    msvc = 'msvc' in conf.env.CC_NAME
    if msvc:
        cflags = []
    else:
        cflags = ['-m32', '-march=i686', '-Wall', '-g']
    conf.env.append_value('CFLAGS', cflags)
    conf.env.append_value('CXXFLAGS', cflags)

# Lazy way to split list of paths, names is input and split_these are paths that are to be
# moved to a separate list.
# split_files(['a', 'b', 'c', 'asd'], ['a']) would return (['b', 'c'], ['a', 'asd'])
def split_files(names, split_these):
    second = []
    for name in split_these:
        second += [path for path in names if name in path.srcpath()]
        names = [path for path in names if not name in path.srcpath()]
    return names, second

def build(bld):
    if bld.options.debug and bld.options.nodebug:
        bld.fatal('Both --debug and --nodebug were specified')
    debug = (bld.env.debug or bld.options.debug) and not bld.options.nodebug
    if bld.options.static and bld.options.nostatic:
        bld.fatal('Both --static and --nostatic were specified')
    optimize = bld.env.optimize or bld.options.optimize
    static = (bld.options.static or bld.env.static) and not bld.options.nostatic
    msvc = 'msvc' in bld.env.CC_NAME
    clang = 'clang' in bld.env.CXX_NAME

    cflags = []
    cxxflags = []
    except_cxxflags = []
    noexcept_cxxflags = []
    linkflags = []
    defines = ['NOMINMAX']
    noexcept_defines = []
    includes = []
    libs = []
    stlibs = []
    libpath = []
    stlibpath = []
    if not msvc:
        cflags += ['-Wno-format', '-Wno-missing-braces']
        if bld.env.CXX_NAME == 'gcc':
            cflags += ['-Wno-strict-overflow', '-Wno-sign-compare']
        # Only msvc works without optimizations atm
        cflags += ['-O3']
        cxxflags += ['--std=gnu++14']
        noexcept_cxxflags += ['-fno-exceptions']
        linkflags += ['-m32', '-pthread', '-Wl,--shared']
        if debug:
            linkflags += ['-Wl,--image-base=0x42300000']
        else:
            cflags += ['-ffunction-sections', '-fdata-sections']
            linkflags += ['-Wl,--gc-sections']
        if static:
            linkflags += ['-static']
            bld.env.SHLIB_MARKER = ''
    else:
        libs += ['user32']
        except_cxxflags += ['/EHsc']
        noexcept_defines += ['_HAS_EXCEPTIONS=0']
        cxxflags += ['/wd4624'] # Silence a seemingly incorrect warning in game.cpp
        cxxflags += ['/Zi', '/FS']
        linkflags += ['/DEBUG']
        if not debug:
            linkflags += ['/OPT:REF', '/OPT:ICF']

    if debug:
        defines += ['DEBUG']

    if not bld.env.freetype_pkg:
        libs += ['freetype']
    ft_inc = bld.options.ft_inc or bld.env.ft_inc
    ft_lib = bld.options.ft_lib or bld.env.ft_lib
    ft_link = bld.options.ft_link or bld.env.ft_link
    if ft_inc:
        node = bld.path.find_node(ft_inc)
        if node:
            includes += [node.abspath()]
        else:
            includes += [ft_inc]
    if ft_lib:
        node = bld.path.find_node(ft_lib)
        if node:
            libpath += [node.abspath()]
        else:
            libpath += [ft_lib]
    if ft_link:
        # Obv ugly
        ft_libs = filter(lambda x: x.startswith('-l'), ft_link.split(' '))
        linkflags += list(filter(lambda x: not x.startswith('-l'), ft_link.split(' ')))
        libs += list(map(lambda x: x[2:], ft_libs))

    if static:
        stlibs = libs
        stlibpath = libpath
        libs = []
        libpath = []

    cxxflags += cflags

    src_with_exceptions = ['save.cpp', 'patchmanager.cpp']

    src = bld.path.ant_glob('src/*.cpp')
    src += bld.path.ant_glob('src/console/*.cpp')
    src += bld.path.ant_glob('src/common/*.cpp')
    src += bld.path.ant_glob('src/patch/*.cpp')
    src, exception_src = split_files(src, ['save', 'patchmanager'])
    # Have to console.cpp as split_files does just a simple substring match atm
    # and it would match all files in console (No, the files aren't organized logically)
    src, console_src = split_files(src, ['scconsole', 'console.cpp', 'cmdargs', 'genericconsole', 'font'])
    src += console_src

    if msvc:
        def_file = ['msvc.def'] # Why doesn't msvc allow @4 prefix on stdcall ???
    else:
        def_file = ['gcc.def']

    bld.shlib(source='', linkflags=linkflags, target='ai_debug', use='obj_with_exceptions obj',
        uselib='freetype', defs=def_file, features='cxx cxxshlib', install_path=None, lib=libs, stlib=stlibs,
        libpath=libpath, stlibpath=stlibpath)

    bld.objects(source=src, cflags=cflags, cxxflags=cxxflags + noexcept_cxxflags,
            defines=defines + noexcept_defines, includes=includes, target='obj')
    bld.objects(source=exception_src, cxxflags=cxxflags + except_cxxflags, defines=defines, includes=includes, target='obj_with_exceptions')
    if not msvc:
        bld(rule='objcopy -S ${SRC} ${TGT}', source='ai_debug.qdp', target='ai_debug_s.qdp')

    bld.install_as('${PREFIX}/ai_debug.qdp', 'ai_debug_s.qdp')
