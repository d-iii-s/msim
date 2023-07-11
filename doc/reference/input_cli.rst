Input command line
==================

The command line support various nice features like command history,
tab completion for completing the commands and arguments while typing, etc.

A very brief overview of the input command line properties:

* Commands and their arguments are separated via whitespace.
  Whitespace characters are: space, tab, period.
* Numeric values can be prefixed by ``0x`` representing hexadecimal notation.
* Numeric values can be suffixed by
  ``k`` and ``K`` (representing multiplication by 1000 and 1024)
  and ``M`` (representing multiplication by 1048576).
* String values should be surrounded by quotes (as in ``"abcd"``).
