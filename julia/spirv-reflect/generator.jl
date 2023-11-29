using Clang.Generators

include_dir = normpath("C:/src/VulkanSDK/1.3.231.1/Source/SPIRV-Reflect", "include")

options = load_options("./generator.toml")

# add compiler flags, e.g. "-DXXXXXXXXX"
args = get_default_args()  # Note you must call this function firstly and then append your own flags
push!(args, "-I$include_dir")

headers = ["C:/src/VulkanSDK/1.3.231.1/Source/SPIRV-Reflect/spirv_reflect.h"]
# there is also an experimental `detect_headers` function for auto-detecting top-level headers in the directory
# headers = detect_headers(clang_dir, args)

# create context
ctx = create_context(headers, args, options)

# run generator
build!(ctx)