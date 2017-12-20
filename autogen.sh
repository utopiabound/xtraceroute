#! /bin/sh
set -x

rm -f config.cache

libtoolize -q
aclocal -I m4
autoheader
#add --include-deps if you want to bootstrap with any other compiler than gcc
#automake --add-missing --copy --include-deps
automake --add-missing --force-missing --copy --include-deps
autoconf
