#! /bin/sh -ex
# This file is automatically generated with: tup generate meta/buildscripts/darwin-from-darwin.sh
export tup_vardict="$(cd $(dirname $0) && pwd)/tup-generate.vardict"
cd "src/quickjs-libc"
(clang -c quickjs-libc.c -rdynamic -g -o quickjs-libc.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c quickjs-libc.c -rdynamic -g -o quickjs-libc.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../cutils"
(clang -c cutils.c -rdynamic -g -o cutils.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c cutils.c -rdynamic -g -o cutils.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../libbf"
(clang -c libbf.c -rdynamic -g -o libbf.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c libbf.c -rdynamic -g -o libbf.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../libregexp"
(clang -c libregexp.c -rdynamic -g -o libregexp.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c libregexp.c -rdynamic -g -o libregexp.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../libunicode"
(clang ../cutils/cutils.host.o unicode_gen.c -rdynamic -g -o unicode_gen.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf unicode_gen.host.dSYM)
(./unicode_gen.host downloaded libunicode-table.h)
(clang -c libunicode.c -rdynamic -g -o libunicode.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c libunicode.c -rdynamic -g -o libunicode.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../quickjs"
(clang -c quickjs.c -rdynamic -g -o quickjs.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c quickjs.c -rdynamic -g -o quickjs.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../qjsc"
(clang qjsc.c ../cutils/cutils.host.o ../libbf/libbf.host.o ../libregexp/libregexp.host.o ../libunicode/libunicode.host.o ../quickjs/quickjs.host.o ../quickjs-libc/quickjs-libc.host.o -rdynamic -g -o qjsc.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf qjsc.host.dSYM)
(clang qjsc.c ../cutils/cutils.target.o ../libbf/libbf.target.o ../libregexp/libregexp.target.o ../libunicode/libunicode.target.o ../quickjs/quickjs.target.o ../quickjs-libc/quickjs-libc.target.o -rdynamic -g -o qjsc.target -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf qjsc.target.dSYM)
cd "../inspect"
(../qjsc/qjsc.host -c -o inspect.c -m inspect.js)
(clang -c inspect.c -rdynamic -g -o inspect.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c inspect.c -rdynamic -g -o inspect.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../qjscalc"
(../qjsc/qjsc.host -fbignum -c -o qjscalc.c qjscalc.js)
(clang -c qjscalc.c -rdynamic -g -o qjscalc.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c qjscalc.c -rdynamic -g -o qjscalc.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../repl"
(../qjsc/qjsc.host -c -o repl.c -m repl.js)
(clang -c repl.c -rdynamic -g -o repl.host.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
(clang -c repl.c -rdynamic -g -o repl.target.o -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm)
cd "../qjs"
(clang qjs.c ../cutils/cutils.host.o ../libbf/libbf.host.o ../libregexp/libregexp.host.o ../libunicode/libunicode.host.o ../quickjs/quickjs.host.o ../inspect/inspect.host.o ../quickjs-libc/quickjs-libc.host.o ../qjscalc/qjscalc.host.o ../repl/repl.host.o -rdynamic -g -o qjs.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf qjs.host.dSYM)
(clang qjs.c ../cutils/cutils.target.o ../libbf/libbf.target.o ../libregexp/libregexp.target.o ../libunicode/libunicode.target.o ../quickjs/quickjs.target.o ../inspect/inspect.target.o ../quickjs-libc/quickjs-libc.target.o ../qjscalc/qjscalc.target.o ../repl/repl.target.o -rdynamic -g -o qjs.target -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf qjs.target.dSYM)
cd "../sample-program"
(../qjsc/qjsc.host -e -o sum.c -m sum.js)
(clang sum.c ../cutils/cutils.host.o ../libbf/libbf.host.o ../libregexp/libregexp.host.o ../libunicode/libunicode.host.o ../quickjs/quickjs.host.o ../inspect/inspect.host.o ../quickjs-libc/quickjs-libc.host.o -rdynamic -g -o sum.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -I../quickjs-libc -ldl -lpthread -lm && rm -rf sum.host.dSYM)
(clang sum.c ../cutils/cutils.target.o ../libbf/libbf.target.o ../libregexp/libregexp.target.o ../libunicode/libunicode.target.o ../quickjs/quickjs.target.o ../inspect/inspect.target.o ../quickjs-libc/quickjs-libc.target.o -rdynamic -g -o sum.target -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -I../quickjs-libc -ldl -lpthread -lm && rm -rf sum.target.dSYM)
cd "../stack-limit-test"
(../qjsc/qjsc.host -c -o loop.c -m loop.js)
(clang main.c loop.c ../cutils/cutils.host.o ../libbf/libbf.host.o ../libregexp/libregexp.host.o ../libunicode/libunicode.host.o ../quickjs/quickjs.host.o ../inspect/inspect.host.o ../quickjs-libc/quickjs-libc.host.o -rdynamic -g -o test.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf test.host.dSYM)
(clang main.c loop.c ../cutils/cutils.target.o ../libbf/libbf.target.o ../libregexp/libregexp.target.o ../libunicode/libunicode.target.o ../quickjs/quickjs.target.o ../inspect/inspect.target.o ../quickjs-libc/quickjs-libc.target.o -rdynamic -g -o test.target -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf test.target.dSYM)
cd "../run-test262"
(clang run-test262.c ../cutils/cutils.host.o ../libbf/libbf.host.o ../libregexp/libregexp.host.o ../libunicode/libunicode.host.o ../quickjs/quickjs.host.o ../inspect/inspect.host.o ../quickjs-libc/quickjs-libc.host.o -rdynamic -g -o run-test262.host -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf run-test262.host.dSYM)
(clang run-test262.c ../cutils/cutils.target.o ../libbf/libbf.target.o ../libregexp/libregexp.target.o ../libunicode/libunicode.target.o ../quickjs/quickjs.target.o ../inspect/inspect.target.o ../quickjs-libc/quickjs-libc.target.o -rdynamic -g -o run-test262.target -D__APPLE__ -DCONFIG_VERSION="\"suchipi-`git rev-parse --short HEAD`\"" -DCONFIG_BIGNUM -DCONFIG_PREFIX="\"/usr/local\"" -DCONFIG_ALL_UNICODE -funsigned-char -Wno-unused-command-line-argument -fPIC -Wall -ldl -lpthread -lm && rm -rf run-test262.target.dSYM)
cd "../archives/core"
(ar -rcs quickjs-core.host.a ../../cutils/cutils.host.o ../../libbf/libbf.host.o ../../libregexp/libregexp.host.o ../../libunicode/libunicode.host.o ../../quickjs/quickjs.host.o)
(ar -rcs quickjs-core.target.a ../../cutils/cutils.target.o ../../libbf/libbf.target.o ../../libregexp/libregexp.target.o ../../libunicode/libunicode.target.o ../../quickjs/quickjs.target.o)
cd "../full"
(ar -rcs quickjs-full.host.a ../../cutils/cutils.host.o ../../libbf/libbf.host.o ../../libregexp/libregexp.host.o ../../libunicode/libunicode.host.o ../../quickjs/quickjs.host.o ../../inspect/inspect.host.o ../../quickjs-libc/quickjs-libc.host.o)
(ar -rcs quickjs-full.target.a ../../cutils/cutils.target.o ../../libbf/libbf.target.o ../../libregexp/libregexp.target.o ../../libunicode/libunicode.target.o ../../quickjs/quickjs.target.o ../../inspect/inspect.target.o ../../quickjs-libc/quickjs-libc.target.o)
