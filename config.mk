
PYTHON_USE=n
PYTHON_PREFIX_INC=/usr/include/python2.3
PYTHON_PREFIX_LIB=/usr/lib
PYTHON_BIN=/usr/bin/python2.3
BOOST_PREFIX_LIB=/usr/lib
BOOST_PREFIX_INC=/usr/include

GSL_USE=n
GSL_PREFIX_INC=/usr/include/gsl
GSL_PREFIX_LIB=/usr/lib

TCP_USE=n

TC_USE=n
ifeq ($(TC_USE),y)
TC_PREFIX_LIB=/usr/lib
TC_LDFLAGS=-L$(TC_PREFIX_LIB) -lpthread -ltcmalloc
else
TC_LDFLAGS=
endif
