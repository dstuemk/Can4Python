import os, sys

from distutils.core import setup, Extension
from distutils import sysconfig

cpp_args = ['-std=c++11', '-stdlib=libc++', '-mmacosx-version-min=10.7']

sfc_module = Extension(
    'Can4Python', sources = ['module.cpp', 'hidapi/hid.c', 'Devices.cpp', 'CanInterface.cpp'],
    include_dirs=['pybind11/include', os.environ['BOOST_ROOT']],
    library_dirs=[os.environ['BOOST_LIBRARYDIR']],
    libraries=['Setupapi'],
    language='c++',
    extra_compile_args = cpp_args,
    )

setup(
    name = 'Can4Python',
    version = '1.0',
    description = 'Python package for sending and receiving CAN Messages',
    ext_modules = [sfc_module],
)