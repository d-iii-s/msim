Documentation for MSIM
======================

This directory contains documentation that is (from master branch)
pushed to <https://msim.readthedocs.io>.

To test the documentation locally, create a virtual environment
for Sphinx and build the HTML version using the commands below.

    python3 -m venv venv-msim-doc
    . ./venv-msim-doc/bin/activate
    pip install -r requirements-doc.txt
    make html
    # Open _build/html/index.html
