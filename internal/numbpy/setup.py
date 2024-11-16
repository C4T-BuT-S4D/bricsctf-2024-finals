from setuptools import setup, Extension

module = Extension(
    "numb",
    sources=["numb.c"],
)

setup(
    name="numb",
    version="0.1",
    description="A module for matrix and vector operations",
    ext_modules=[module],
)
