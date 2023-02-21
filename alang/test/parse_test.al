module HelloWorld

import std.core.io, dump

func one() : I32
	return 1
end

func inc(x : Ref{I32})
	x = x + 1
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

	io.print("Hello, friend!")
	std.core.io.debug("$(a) + $(b) = $(c)")
	if c % 2 == 0 && !(b > 32)
		c = -c*a - b^+2^3 - b + 1
	end
	&p = &c
	dump(5)
end

end