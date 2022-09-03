from setuptools import setup, Extension

setup(
    name='mykmeanssp',
    version='2.0.0',
    author="Rotem Zluf, Naama Tsiben",
    description="K-means clustering algorithm",
    ext_modules=[
        Extension(
            'mykmeanssp',
            ['spkmeans.c','spkmeansmodule.c'],
        ),
    ]
)
