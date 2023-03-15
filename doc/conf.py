#
# readthedocs configuration
#

from __future__ import division, print_function, unicode_literals

from datetime import datetime

from recommonmark.parser import CommonMarkParser

extensions = []
source_suffix = ['.rst', '.md']
source_parsers = {
    '.md': CommonMarkParser,
}

master_doc = 'index'
project = u'MSIM'
copyright = '2000 - 2023'
version = 'stable'
release = 'stable'
pygments_style = 'sphinx'
htmlhelp_basename = 'msim'
html_theme = 'sphinx_rtd_theme'
html_sidebars = { '**': ['globaltoc.html', 'relations.html', 'sourcelink.html', 'searchbox.html'] }
exclude_patterns = ['_build', 'venv-*', 'tests']