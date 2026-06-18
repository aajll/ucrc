# Configuration file for the Sphinx documentation builder.
#
# For all the built-in options, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

import os
import sys

sys.path.insert(0, os.path.abspath("."))

# -- Project information -----------------------------------------------------

project = "ucrc"
copyright = "2026, ucrc Contributors"
author = "ucrc Contributors"

release = "0.1.0"

# -- General configuration ---------------------------------------------------

extensions = [
    "breathe",
    "sphinx.ext.mathjax",
    "sphinx.ext.autodoc",
    "sphinx.ext.intersphinx",
]

templates_path = ["_templates"]

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

source_suffix = ".rst"

master_doc = "index"

language = "en"

# -- Options for HTML output -------------------------------------------------

html_theme = "alabaster"
html_theme_options = {
    "description": "Lean CRC-8/16/32 library for embedded, RTOS, and Linux targets",
}

html_static_path = ["_static"]

# -- Options for Breathe -----------------------------------------------------

breathe_default_project = "ucrc"
breathe_default_domain = "c"

# -- Options for intersphinx -------------------------------------------------

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}
