from setuptools import setup, find_packages, Extension

setup(
    name="mykmeanssp",
    version=1,
    description="c api for kmeans algorithm",
    packages= find_packages(),
    classifiers=[
            'Development Status :: 3 - Alpha',
            'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
            'Natural Language :: English',
            'Programming Language :: Python :: 3 :: Only',
            'Programming Language :: Python :: Implementation :: CPython',
        ],
    ext_modules= [
        Extension(
            "mykmeanssp",
            ['spkmeansmodule.c'],
        ),
    ]
)
