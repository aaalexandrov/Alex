module parse.test

import std.io, std

module nested
	const pi : F32 = 3.14
end

func one() : I32
	return 1
end

const inc_by : I32 = 33
var last_inc : I32 = 0

func inc(x : Ref{I32})
	x = x + inc_by
	last_inc = x
end

func add1(x : I32) : I32
	return x + 1
end

func main()
	var a : I32 = 'a'
	const b : I32 = 42 # the ANSWER
	var c : I32 = a + b
	var f : F32 = c * 5.25
	var p : Ref{I32} = &a
	var arr : Array{I32, 5}
	const o : I32 = one()

	arr[0] = c
	var i : I32 = 1
	while i < 5
		arr[i] = arr[i - 1] + 1
	end

	print("Hello, friend!")
	std.io.print("$(a) + $(b) = $(c)")
	if c % 2 == 0 && !(b > 32)
		c = -c*a - b^+2^3 - b + 1
	end
	&p = &c
	add1(5)
end

end