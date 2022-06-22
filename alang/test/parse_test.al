module HelloWorld

import io

func main()
	var a : I32 = 'a'
	const b : I32 = 42 # the ANSWER
	var c : I32 = a + b
	var f : F32 = c * 5.25
	io.print("Hello, friend!")
	io.print("$(a) + $(b) = $(c)")
end

end