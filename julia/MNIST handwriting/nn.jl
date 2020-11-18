module NN

using Random, LinearAlgebra

mutable struct NNLayer{Num<:Real}
    weights::Matrix{Num}
    outFunc::Function
    outFuncDerivative::Function
    alpha::Num
    dropOutRetainCoef::Num
end

function evaluate(layer::NNLayer{Num}, input::AbstractMatrix{Num})::AbstractMatrix{Num} where {Num<:Real}
    layer.outFunc.(layer.weights * input)
end

mutable struct NNet{Num <: Real}
    layers::Vector{NNLayer{Num}}
end

relu(x) = (x > 0) * x
relu_derivative(x) = (x > 0)

sigmoid(x) = 1/(1+exp(-x))
function sigmoid_derivative(x)
    e = exp(-x)
    e / (1+e)^2
end

tanh_derivative(x) = 1-tanh(x)^2

randsym(dims...) = rand(dims...) .* 2 .- 1

function NNet(layerSizes::Vector{Int}, alpha::Num=Num(0.1), outFunc::Function=relu, outFuncDerivative::Function=relu_derivative; initFunc = randsym) where {Num <: Real}
    nn = NNet{Num}(Vector{NNLayer{Num}}())
    for l = 2:length(layerSizes)
        func, deriv = l < length(layerSizes) ? (outFunc, outFuncDerivative) : (identity, one)
        layer = NNLayer{Num}(initFunc(layerSizes[l], layerSizes[l-1]), func, deriv, alpha, Num(1.0))
        push!(nn.layers, layer)
    end
    nn
end

function evaluate(nnet::NNet{Num}, input::AbstractMatrix{Num})::AbstractMatrix{Num} where {Num<:Real}
    v = input
    for l in nnet.layers
        v = evaluate(l, v)
    end
    v
end

function back_propagate(nnet::NNet{Num}, input::AbstractMatrix{Num}, goalOutput::AbstractMatrix{Num}) where {Num<:Real}
    layerValues = [input]
    layerDropOutMasks = Vector{Bool}[]
    for layer in nnet.layers
        layerEval = evaluate(layer, last(layerValues))
        dropOutMask = rand(size(layerEval, 1)) .<= layer.dropOutRetainCoef
        layerEval .*= dropOutMask ./ layer.dropOutRetainCoef
        push!(layerValues, layerEval)
        push!(layerDropOutMasks, dropOutMask)
    end

    layerDeltas = Vector{Matrix{Num}}(undef, length(nnet.layers))
    lastLayer = nnet.layers[end]
    layerDeltas[end] = (goalOutput .- layerValues[end]) .* lastLayer.outFuncDerivative.(layerValues[end])
    for l = length(layerDeltas)-1:-1:1
        inLayer = nnet.layers[l]
        outLayer = nnet.layers[l+1]
        layerDeltas[l] = (outLayer.weights' * layerDeltas[l+1]) .* inLayer.outFuncDerivative.(layerValues[l+1])
        layerDeltas[l] .*= layerDropOutMasks[l]
    end

    batchSize = size(input, 2)
    for l = 1:length(nnet.layers)
        layer = nnet.layers[l]
        layer.weights .+= layer.alpha / batchSize .* layerDeltas[l] * layerValues[l]'
    end
    nnet
end

function train(nnet::NNet{Num}, inputs::Matrix{Num}, goalOutputs::Matrix{Num}; iterations::Int = 100, batchSize::Int = 1) where {Num<:Real}
    for i = 1:iterations
        perm = randperm(size(inputs, 2))
        for c = 1:batchSize:length(perm)
            colRange = c:min(c + batchSize - 1, length(perm))
            back_propagate(nnet, inputs[:, colRange], goalOutputs[:, colRange])
        end
    end
    nnet
end

function test_xor(hiddenLayers = [4], alpha = 0.2, iterations = 60, initFunc = randsym)
    inputs = Float64[1 0 0 1;
                     0 1 0 1;
                     1 1 1 1]
    outputs = Float64[1 1 0 0;]
    nn = NNet([3, hiddenLayers..., 1], alpha; initFunc = initFunc)
    train(nn, inputs, outputs; iterations = iterations)
    error = 0.0
    for c = 1:size(inputs, 2)
        result = evaluate(nn, inputs[:, c:c])
        error += sum((result - outputs[:, c:c]).^2)
    end
    nn, error
end

end
