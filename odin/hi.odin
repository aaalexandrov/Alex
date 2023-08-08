package main

import "core:fmt"
import "core:io"
import "core:os"
import win32 "core:sys/windows"
import "btree"

fib :: proc(n: i64) -> i64 {
    a, b: i64 = 0, 1
    for i in 1..=n {
        a, b = b, a+b
    }
    return a
}

fib_rec :: proc(n: i64) -> i64 {
    if n <= 1 { 
        return n
    }
    return fib_rec(n-2) + fib_rec(n-1)
}

main :: proc() {
    fmt.println("Hi!")
    fmt.println(fib(16))
    fmt.println(fib_rec(16))

    // buf : [1024]u8
    // i, err := os.read(os.stdin, buf[:])
    // fmt.println("Read ", buf[:i])

    vals1 : []int = { 0, 0, 5, 10, 15, 15, 15, 30, 30, 40, 45, 50, 50 }
    v :: 50
    l := btree.lower_bound(vals1, v)
    u := btree.upper_bound(vals1, v)
    assert(0 <= l && l <= len(vals1))
    assert(0 <= u && u <= len(vals1))
    assert(l <= u)
    fmt.println("Lower bound of ", v, ": ", l, "prev value: ", vals1[max(0, l-1)], ", value: ", vals1[min(l, len(vals1)-1)])
    fmt.println("Upper bound of ", v, ": ", u, "prev value: ", vals1[max(0, u-1)], ", value: ", vals1[min(u, len(vals1)-1)])

    fmt.println("kekek, bye!")
}