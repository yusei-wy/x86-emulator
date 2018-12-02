#!/bin/sh

# 引数が1個じゃんければエラー
if [ $# -ne 1 ]; then
  echo "./compile_func [c filename(not extension)]"
  exit 1
fi

gcc -Wl,--entry=func,--oformat=binary -nostdlib -fno-asynchronous-unwind-tables -o $1.bin $1.c
