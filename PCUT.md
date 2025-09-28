# How to add pcut dep

1. add `pcut` repo as a submodule
```shell
git submodule add https://github.com/vhotspur/pcut.git
```

2. build pcut as per instruction in its [README](pcut/README.rst)

3. use modified [Makefile](tests/rvtests/unit-tests/Makefile) with correct flags to include pcut headers and link pcut library

Now `make test` should work