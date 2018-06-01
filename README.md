# Ignotum

a simple lib to help read and write in mapped memory regions of a linux process

### Compiling:

```
make # create ignotum.o and libignotum.so
make install # install the lib
make test # optional, compile the tests
```

#### Options:

```
CC - compiler (Default: gcc)
INSTALL_LIB_DIR - dir to install .so file (Default: /usr/lib64)
INSTALL_HEADER_DIR - dir to install .h file (Default: /usr/include)
```

### Documentation:

see the man files at doc/
