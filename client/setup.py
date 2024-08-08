# setup.py
from setuptools import setup, find_packages

setup(
    name="fix-client",
    version="0.1.0",
    packages=find_packages(),
#     install_requires=[
# #        "quickfix",
#     ],
    entry_points={
        "console_scripts": [
            "fix-client = app.client:main",
        ]
    },
)
