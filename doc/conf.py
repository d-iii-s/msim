#
# readthedocs configuration
#

from __future__ import division, print_function, unicode_literals

from datetime import datetime

from pygments.lexer import RegexLexer
from pygments import token
from recommonmark.parser import CommonMarkParser
from sphinx.highlighting import lexers

class MsimLexer(RegexLexer):
    name = 'msim'
    tokens = {
        'root': [
            (r'\#.*', token.Comment.Single),
            (r'\[msim\]', token.Generic.Prompt),
            (r'\b(add|quit|dumpmem|dumpins|dumpdev|dumpphys|break|dumpbreak|rembreak|stat|echo|continue|step|set|unset|help)\b', token.Keyword),
            (r'[a-zA-Z][a-zA-Z_0-9]*', token.Name),
            (r'0x[0-9a-fA-F]*', token.Literal.Number),
            (r'\b[0-9][0-9]*[kM]\b', token.Literal.Number),
            (r'[0-9][0-9]*', token.Literal.Number),
            (r'"[^"]*"', token.Literal.String),
            (r'\s', token.Text),
            (r'[-\[\].,()=:&]', token.Operator),
            (r'<[^>]*>', token.Generic.Error),
        ]
    }


lexers['msim'] = MsimLexer(startinline=True)

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