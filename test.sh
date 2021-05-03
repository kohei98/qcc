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

assert 3 'main() { if (0) return 2; return 3; }'
assert 3 'main() { if (1-1) return 2; return 3; }'
assert 2 'main() { if (1) return 2; return 3; }'
assert 2 'main() { if (2-1) return 2; return 3; }'
assert 55 'main() { i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'main() { for (;;) return 3; return 5; }'
assert 10 'main() { i=0; while(i<10) i=i+1; return i; }'
assert 3 'main() { {1; {2;} return 3;} }'
assert 5 'main() { ;;; return 5; }'
assert 10 'main() { i=0; while(i<10) i=i+1; return i; }'
assert 55 'main() { i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 5 'foo(x,y){return x + y;} main() {return foo(3,2);}'
assert 2 'main(){return 2;}'
assert 7 'main(){return 3 + 4;}'
assert 1 'main(){a = 1;if(4 > 2){if(a < 0){a = a * 5;} return a;}}'
assert 14 'foo(x,y,z){return x + y + z;}main(){return foo(3,6,5);}'
assert 7 'main(x,y){x = 3;y = 4;return x + y;}'
assert 21 'fib(x){if(x <= 1){return 1;}else {return fib(x - 1) + fib(x - 2);}} main(){a = 0;return fib(7);}'
assert 32 'main() { return ret32(); } ret32() { return 32; }'
assert 7 'main() { return add2(3,4); } add2(x, y) { return x+y; }'
assert 1 'main() { return sub2(4,3); } sub2(x, y) { return x-y; }'
assert 55 'main() { return fib(9); } fib(x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

echo OK