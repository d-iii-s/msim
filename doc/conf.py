#
# readthedocs configuration
#

from __future__ import division, print_function, unicode_literals

from datetime import datetime

from pygments.lexer import RegexLexer
from pygments import token
from recommonmark.parser import CommonMarkParser
from sphinx.highlighting import lexers
from sphinx.util.docutils import SphinxDirective
from docutils.parsers.rst.directives import admonitions


class ArchBoxDirective(admonitions.Admonition, SphinxDirective):
    required_arguments = 1
    has_content = True

    def run(self) -> list[nodes.Node]:
        self.assert_has_content()

        self.arguments = ["{} specific notes".format(self.arguments[0])]

        ret = super().run()
        return ret

class ExtrasBoxDirective(admonitions.Admonition, SphinxDirective):
    required_arguments = 1
    has_content = True

    def run(self) -> list[nodes.Node]:
        self.assert_has_content()

        self.arguments = ["Extra information ({})".format(self.arguments[0])]

        ret = super().run()
        return ret

class QuizBoxDirective(admonitions.Admonition, SphinxDirective):
    required_arguments = 0
    has_content = True

    def run(self) -> list[nodes.Node]:
        self.assert_has_content()

        self.arguments = ["Self-test quiz"]

        ret = super().run()
        return ret


def setup(app: Sphinx) -> ExtensionMetadata:
    app.add_directive('archbox', ArchBoxDirective)
    app.add_directive('extras', ExtrasBoxDirective)
    app.add_directive('quiz', QuizBoxDirective)

    return {
        'version': '0.1',
        'parallel_read_safe': True,
        'parallel_write_safe': True,
    }


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
            (r'\b[0-9a-fA-F][0-9a-fA-F]*\b', token.Literal.Number),
            (r'[0-9][0-9]*', token.Literal.Number),
            (r'\b(T|F)\b', token.Literal),
            (r'"[^"]*"', token.Literal.String),
            (r'\s', token.Text),
            (r'[-\[\].,()=:&\^\+\*<>/]', token.Operator),
            (r'<[A-Z][^>]*>', token.Generic.Error),
            (r'<[^>]*>', token.Text),
        ]
    }


lexers['msim'] = MsimLexer(startinline=True)

extensions = [
    'sphinx_toolbox.collapse',
    'sphinx_code_tabs',
]
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
