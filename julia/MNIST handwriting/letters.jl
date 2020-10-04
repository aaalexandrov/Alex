using Random, Plots

function read_array(path)
    open(path) do io
        magic0 = read(io, UInt8)
        magic1 = read(io, UInt8)
        if magic0 != 0 || magic1 != 0
            error("Invalid format")
        end
        elemTypeId = read(io, UInt8)
        types = Dict(0x08=>UInt8, 0x09=>Int8, 0x0b=>Int16, 0x0c=>Int32, 0x0d=>Float32, 0x0e=>Float64)
        elemType = types[elemTypeId]
        dims = read(io, UInt8)
        dimVec = Vector{Int32}(undef, dims)
        for d = dims:-1:1
            dimVec[d] = ntoh(read(io, Int32))
        end
        arr = Array{elemType}(undef, dimVec...)
        read!(io, arr)
        map!(ntoh, arr, arr)
    end
end

function load_letter_arrays(namePrefix)
    images = read_array("$namePrefix-images-idx3-ubyte")
    labels = read_array("$namePrefix-labels-idx1-ubyte")
    images, labels
end

function load_letters(namePrefix)
    images, labels = load_letter_arrays(namePrefix)
    collect(zip((reverse(images[:,:,i]', dims=1) for i=1:size(images, 3)), labels))
end

function plot_letter(letters, i)
    println(letters[i][2])
    plot(contourf(letters[i][1]))
end

function adjust_weights(weights, alpha, inputs, goalOutputs)
    outputs = weights * inputs
    deltas = outputs .- goalOutputs
    #errors = deltas .^ 2
    adj = inputs' .* deltas # outer product
    weights .-= adj .* alpha
end

function get_goal_outputs(letter)
    value = letter[2]
    outputs = zeros(Float64, 10)
    outputs[value + 1] = 1.0
    outputs
end

function adjust_for_letter(weights, alpha, letter)
    inputs = letter[1][:] ./ 255.0
    outputs = get_goal_outputs(letter)
    adjust_weights(weights, alpha, inputs, outputs)
end

function train(letters; alpha = 0.001, trainRuns = 10)
    pixSize = length(letters[1][1])
    weights = zeros(10, pixSize) # zeros works better than rand
    for i = 1:trainRuns
        for l in shuffle(letters)
            adjust_for_letter(weights, alpha, l)
        end
    end
    weights
end

function classify_letter(weights, letter)
    inputs = letter[1][:] ./ 255.0
    outputs = weights * inputs
    prob, ind = findmax(outputs)
    ind - 1
end

function test_letter(weights, letter)
    val = classify_letter(weights, letter)
    expectedVal = letter[2]
    val == expectedVal
end

function test_letters(weights, letters)
    correct = 0
    for l in letters
        correct += test_letter(weights, l)
    end
    correct / length(letters)
end

function test(trainName, testName; alpha = 0.001, trainRuns = 10)
    letters = load_letters(trainName)
    weights = train(letters; alpha=alpha, trainRuns=trainRuns)
    testLetters = load_letters(testName)
    test_letters(weights, testLetters)
end

include("nn.jl")
function load_letters_nn(name, limit = -1)
    images, labels = load_letter_arrays(name)
    images = hcat((images[:, :, i][:]./255.0 for i = 1:size(images, 3))...)
    minLabel, maxLabel = extrema(labels)
    labels = hcat(([(i == l) * 1.0 for i = minLabel:maxLabel] for l in labels)...)
    if limit > 0 && limit < size(images, 2)
        images = images[:, 1:limit]
        labels = labels[:, 1:limit]
    end
    images, labels
end

function test_nn(;hiddenLayers = [40], dropOuts=[1=>0.5], alpha = 0.005, limitTrain = 1000, limitTest = 1000, trainRuns = 10, batchSize = 1)
    trainImages, trainLabels = load_letters_nn("train", limitTrain)
    pixSize = size(trainImages, 1)
    numLabels = size(trainLabels, 1)
    nn = NN.NNet([pixSize, hiddenLayers..., numLabels], alpha; initFunc=(x,y)->0.2*rand(x,y).-0.1)
    for (l,d) in dropOuts
        nn.layers[l].dropOutRetainCoef = d
    end
    NN.train(nn, trainImages, trainLabels; iterations = trainRuns, batchSize = batchSize)
    testImages, testLabels = load_letters_nn("t10k", limitTest)
    #testImages, testLabels = trainImages, trainLabels
    correct = 0
    for t = 1:size(testImages, 2)
        out = NN.evaluate(nn, testImages[:, t:t])
        outNum = findmax(out)[2][1]
        testNum = findmax(testLabels[:, t])[2]
        correct += outNum == testNum
    end
    nn, correct / size(testImages, 2)
end

#println(test("train", "t10k"))
