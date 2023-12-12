#!/bin/bash

srcdir=`dirname $0`
test -n "$srcdir" || srcdir=.

cd "$srcdir"

./autoclean.sh

function failed {
    cat << EOF 1>&2

Result: $1 failed

Please try running the commands from autogen.sh manually, and fix errors.
EOF
    exit 1
}

if test `uname -s` = Linux -o `uname -s` = FreeBSD; then
    libtoolize || failed "libtool"
elif test `uname -s` = Darwin; then
    libtoolize || glibtoolize || failed "Can't find libtoolize or glibtoolize. Use lode or install it yourself."
fi

aclocal || failed "aclocal"

autoheader || failed "autoheader"

automake --add-missing || failed "automake"

autoreconf || failed "autoreconf"

cat << EOF

Result: All went OK, please run $srcdir/configure (with the appropriate parameters) now.

EOF

