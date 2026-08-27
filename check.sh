#!/bin/bash


function CheckCompile
{
echo -n "compile with: $1...   "

make clean &> /dev/null
./configure $1 &> /dev/null
make &> /dev/null


if [ -e libUseful.a ]
then
  echo PASSED
else
  echo FAIL
fi

}


CheckCompile "--disable-ssl --disable-seccomp --disable-namespaces"
CheckCompile "--enable-ssl --disable-seccomp --disable-namespaces"
CheckCompile "--enable-seccomp --disable-namespaces"
CheckCompile "--enable-seccomp --enable-namespaces"
CheckCompile "--enable-seccomp --disable-capabilities"
CheckCompile "--enable-seccomp --enable-capabilites"
CheckCompile "--enable-seccomp --disable-smtp"
CheckCompile "--enable-seccomp --enable-smtp"

