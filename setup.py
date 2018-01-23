#!/usr/bin/env python

import numpy as np
import os
from setuptools import setup, Extension

VERSION = "0.1.0"

README = open('README.md').read()

implementation = Extension(
    name="seeded_zwatershed._watershed",
    language="c++",
    sources=[os.path.join("src", _) for _ in 
             "szw.cpp", "_watershed.pyx"],
    extra_compile_args=['-std=c++11'],
    include_dirs=["include", np.get_include()])
             
setup(
    name="seeded_zwatershed",
    version=VERSION,
    packages=["seeded_zwatershed"],
    url="https://github.com/vcg/seeded_watershed",
    description="A seeded watershed using affinities for connection weights",
    long_description=README,
    install_requires=[
        "Cython",
        "numpy"
    ],
    ext_modules=[implementation],
    zip_safe=False
)
