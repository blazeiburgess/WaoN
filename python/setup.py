#!/usr/bin/env python
"""Setup script for WaoN Python bindings."""

from setuptools import setup, find_packages
import os
import sys

# Read long description from README if it exists
long_description = ""
# Try parent directory first, then current directory
readme_paths = [
    os.path.join(os.path.dirname(__file__), "../README.md"),
    os.path.join(os.path.dirname(__file__), "README.md")
]
for readme_path in readme_paths:
    if os.path.exists(readme_path):
        with open(readme_path, "r", encoding="utf-8") as f:
            long_description = f.read()
        break

setup(
    name="waon",
    version="@PROJECT_VERSION@",
    author="WaoN Development Team",
    author_email="kichiki@users.sourceforge.net",
    description="Python bindings for WaoN - Wave-to-Notes transcriber",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/blazeiburgess/WaoN",
    packages=find_packages(),
    package_data={
        "waon": ["*.so", "*.pyd", "*.dll"],
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: Multimedia :: Sound/Audio :: Analysis",
        "Topic :: Multimedia :: Sound/Audio :: Conversion",
    ],
    python_requires=">=3.6",
    install_requires=[
        "numpy>=1.16.0",
    ],
    extras_require={
        "dev": [
            "pytest>=6.0",
            "pytest-cov",
        ],
    },
)