#!/usr/bin/env python
# -*- coding: utf-8 -*-

from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

sourcefiles = ['ftreader.pyx', 'lib/ftreader.c']

ext_modules = [Extension('ftreader', sourcefiles,
    libraries=['ft'],
    include_dirs=['include']
    #include_dirs=['/usr/include/gwenhywfar4', '/usr/include/aqbanking5', ],
    #extra_compile_args=['-Wno-cast-qual', '-Wno-strict-prototypes', ],
    )]

setup(
    name = 'flowy',
    version='1.32',
    description='flowy network traffic analyzer',
    long_description='''
put a longer description here
''',
    cmdclass = {'build_ext': build_ext},
    ext_modules = ext_modules,
    download_url='http://pyneo.org/downloads/',
    author='Johannes Schauer',
    author_email='j.schauer@jacobs-university.de',
    url='http://pyneo.org/',
    data_files=[
        ('share/pyneod', ('pybankd.py', )),
        ],
)


