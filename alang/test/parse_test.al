module HelloWorld

import io

func main()
	var a : I32 = 'a'
	const b : I32 = 42 # the ANSWER
	var c : I32 = a + b
	var f : F32 = c * 5.25
	var p : Ref{I32} = &a
	var arr : Array{I32, 5}

	arr[0] = c
	var i : I32 = 1
	while i < 5
		arr[i] = arr[i - 1] + 1
	end

	io.print("Hello, friend!")
	io.print("$(a) + $(b) = $(c)")
	if c % 2 == 0
		c = -c*a - b^+2 + 1
	end
	&p = &c
end

end