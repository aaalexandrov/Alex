module NN

using Random, LinearAlgebra

mutable struct NNLayer{Num<:Real}
    weights::Matrix{Num}
    outFunc::Function
    outFuncDerivative::Function
    alpha::Num
    dropOutRetainCoef::Num
end

function evaluate(layer::NNLayer{Num}, input::Vector{Num})::Vector{Num} where {Num<:Real}
    layer.outFunc.(layer.weights * input)
end

mutable struct NNet{Num <: Real}
    layers::Vector{NNLayer{Num}}
end

relu(x) = (x > 0) * x
relu_derivative(x) = (x > 0)

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

function evaluate(nnet::NNet{Num}, input::Vector{Num})::Vector{Num} where {Num<:Real}
    v = input
    for l in nnet.layers
        v = evaluate(l, v)
    end
    v
end

function back_propagate(nnet::NNet{Num}, input::Vector{Num}, goalOutput::Vector{Num}) where {Num<:Real}
    layerValues = [input]
    layerDropOutMasks = Vector{Bool}[]
    for layer in nnet.layers
        layerEval = evaluate(layer, last(layerValues))
        dropOutMask = rand(length(layerEval)) .<= layer.dropOutRetainCoef
        layerEval .*= dropOutMask ./ layer.dropOutRetainCoef
        push!(layerValues, layerEval)
        push!(layerDropOutMasks, dropOutMask)
    end

    layerDeltas = Vector{Vector{Num}}(undef, length(nnet.layers))
    lastLayer = nnet.layers[end]
    layerDeltas[end] = (goalOutput .- layerValues[end]) .* lastLayer.outFuncDerivative.(layerValues[end])
    for l = length(layerDeltas)-1:-1:1
        inLayer = nnet.layers[l]
        outLayer = nnet.layers[l+1]
        layerDeltas[l] = (outLayer.weights' * layerDeltas[l+1]) .* inLayer.outFuncDerivative.(layerValues[l+1])
        layerDeltas[l] .*= layerDropOutMasks[l]
    end

    for l = 1:length(nnet.layers)
        layer = nnet.layers[l]
        layer.weights .+= layer.alpha .* layerDeltas[l] * layerValues[l]'
    end
    nnet
end

function train(nnet::NNet{Num}, inputs::Matrix{Num}, goalOutputs::Matrix{Num}; iterations::Int = 100) where {Num<:Real}
    for i = 1:iterations, c in randperm(size(inputs, 2))
        back_propagate(nnet, inputs[:, c], goalOutputs[:, c])
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
        result = evaluate(nn, inputs[:, c])
        error += sum((result - outputs[:, c]).^2)
    end
    nn, error
end

end
