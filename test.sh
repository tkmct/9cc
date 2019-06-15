#! /bin/bash
try() {
  expected="$1"
  input="$2"

  gcc -w -o 9cc 9cc.c
  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 '5+20-4;'
try 41 " 12 + 34 - 5;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 5 "a = 5; a;"
try 1 "a = b = 1;"
try 4 "ab = 4; ab;"
try 20 "ab = 5; cd = 4; ab*cd;"
try 1 "_foo = 1; _foo;";
try 1 "a = 1; return a;"

echo OK
