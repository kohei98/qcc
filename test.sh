#!/bin/bash
assert (){
    expected="$1"
    input="$2"

    ./qcc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}
assert 1 'a = 1;if(4 > 2){if(a < 0){a = a * 5;} return a;}'
assert 20 'a = 0;for(;a < 20;a = a + 2)b = 3;return a;'
assert 10 'a = 0; while(a < 10) a = a + 1; return a;'
assert 34 'if(2 >= 45) return 20; else return 34;'
assert 3 'a = 3;b = 4;c = 20;
if (a > b) return a * b + c;
return a;'
assert 20 'if (3 > 2) return 20;'
assert 23 'S = 20;
           T = 2;
           R = 1;
           return S + T + R;'
assert 14 'hoge = 3;foo = 5 * 6 - 8;return hoge + foo / 2;'
assert 3 'return 3;return 34;return 6;'
assert 14 'a = 4;b = 5 * 2;return a + b;'


assert 14 'hoge = 3;foo = 5 * 6 - 8;hoge + foo / 2;'
assert 3 'a = 3;'
assert 14 'a = 3;b = 5 * 6 - 8;a + b / 2;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 10 '- -10;'
assert 10 '- - +10;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'
echo OK