module SpirvReflect

using CEnum

const SpvId = Cuint

@cenum SpvSourceLanguage_::UInt32 begin
    SpvSourceLanguageUnknown = 0
    SpvSourceLanguageESSL = 1
    SpvSourceLanguageGLSL = 2
    SpvSourceLanguageOpenCL_C = 3
    SpvSourceLanguageOpenCL_CPP = 4
    SpvSourceLanguageHLSL = 5
    SpvSourceLanguageCPP_for_OpenCL = 6
    SpvSourceLanguageSYCL = 7
    SpvSourceLanguageMax = 2147483647
end

const SpvSourceLanguage = SpvSourceLanguage_

@cenum SpvExecutionModel_::UInt32 begin
    SpvExecutionModelVertex = 0
    SpvExecutionModelTessellationControl = 1
    SpvExecutionModelTessellationEvaluation = 2
    SpvExecutionModelGeometry = 3
    SpvExecutionModelFragment = 4
    SpvExecutionModelGLCompute = 5
    SpvExecutionModelKernel = 6
    SpvExecutionModelTaskNV = 5267
    SpvExecutionModelMeshNV = 5268
    SpvExecutionModelRayGenerationKHR = 5313
    SpvExecutionModelRayGenerationNV = 5313
    SpvExecutionModelIntersectionKHR = 5314
    SpvExecutionModelIntersectionNV = 5314
    SpvExecutionModelAnyHitKHR = 5315
    SpvExecutionModelAnyHitNV = 5315
    SpvExecutionModelClosestHitKHR = 5316
    SpvExecutionModelClosestHitNV = 5316
    SpvExecutionModelMissKHR = 5317
    SpvExecutionModelMissNV = 5317
    SpvExecutionModelCallableKHR = 5318
    SpvExecutionModelCallableNV = 5318
    SpvExecutionModelMax = 2147483647
end

const SpvExecutionModel = SpvExecutionModel_

@cenum SpvAddressingModel_::UInt32 begin
    SpvAddressingModelLogical = 0
    SpvAddressingModelPhysical32 = 1
    SpvAddressingModelPhysical64 = 2
    SpvAddressingModelPhysicalStorageBuffer64 = 5348
    SpvAddressingModelPhysicalStorageBuffer64EXT = 5348
    SpvAddressingModelMax = 2147483647
end

const SpvAddressingModel = SpvAddressingModel_

@cenum SpvMemoryModel_::UInt32 begin
    SpvMemoryModelSimple = 0
    SpvMemoryModelGLSL450 = 1
    SpvMemoryModelOpenCL = 2
    SpvMemoryModelVulkan = 3
    SpvMemoryModelVulkanKHR = 3
    SpvMemoryModelMax = 2147483647
end

const SpvMemoryModel = SpvMemoryModel_

@cenum SpvExecutionMode_::UInt32 begin
    SpvExecutionModeInvocations = 0
    SpvExecutionModeSpacingEqual = 1
    SpvExecutionModeSpacingFractionalEven = 2
    SpvExecutionModeSpacingFractionalOdd = 3
    SpvExecutionModeVertexOrderCw = 4
    SpvExecutionModeVertexOrderCcw = 5
    SpvExecutionModePixelCenterInteger = 6
    SpvExecutionModeOriginUpperLeft = 7
    SpvExecutionModeOriginLowerLeft = 8
    SpvExecutionModeEarlyFragmentTests = 9
    SpvExecutionModePointMode = 10
    SpvExecutionModeXfb = 11
    SpvExecutionModeDepthReplacing = 12
    SpvExecutionModeDepthGreater = 14
    SpvExecutionModeDepthLess = 15
    SpvExecutionModeDepthUnchanged = 16
    SpvExecutionModeLocalSize = 17
    SpvExecutionModeLocalSizeHint = 18
    SpvExecutionModeInputPoints = 19
    SpvExecutionModeInputLines = 20
    SpvExecutionModeInputLinesAdjacency = 21
    SpvExecutionModeTriangles = 22
    SpvExecutionModeInputTrianglesAdjacency = 23
    SpvExecutionModeQuads = 24
    SpvExecutionModeIsolines = 25
    SpvExecutionModeOutputVertices = 26
    SpvExecutionModeOutputPoints = 27
    SpvExecutionModeOutputLineStrip = 28
    SpvExecutionModeOutputTriangleStrip = 29
    SpvExecutionModeVecTypeHint = 30
    SpvExecutionModeContractionOff = 31
    SpvExecutionModeInitializer = 33
    SpvExecutionModeFinalizer = 34
    SpvExecutionModeSubgroupSize = 35
    SpvExecutionModeSubgroupsPerWorkgroup = 36
    SpvExecutionModeSubgroupsPerWorkgroupId = 37
    SpvExecutionModeLocalSizeId = 38
    SpvExecutionModeLocalSizeHintId = 39
    SpvExecutionModeSubgroupUniformControlFlowKHR = 4421
    SpvExecutionModePostDepthCoverage = 4446
    SpvExecutionModeDenormPreserve = 4459
    SpvExecutionModeDenormFlushToZero = 4460
    SpvExecutionModeSignedZeroInfNanPreserve = 4461
    SpvExecutionModeRoundingModeRTE = 4462
    SpvExecutionModeRoundingModeRTZ = 4463
    SpvExecutionModeStencilRefReplacingEXT = 5027
    SpvExecutionModeOutputLinesNV = 5269
    SpvExecutionModeOutputPrimitivesNV = 5270
    SpvExecutionModeDerivativeGroupQuadsNV = 5289
    SpvExecutionModeDerivativeGroupLinearNV = 5290
    SpvExecutionModeOutputTrianglesNV = 5298
    SpvExecutionModePixelInterlockOrderedEXT = 5366
    SpvExecutionModePixelInterlockUnorderedEXT = 5367
    SpvExecutionModeSampleInterlockOrderedEXT = 5368
    SpvExecutionModeSampleInterlockUnorderedEXT = 5369
    SpvExecutionModeShadingRateInterlockOrderedEXT = 5370
    SpvExecutionModeShadingRateInterlockUnorderedEXT = 5371
    SpvExecutionModeSharedLocalMemorySizeINTEL = 5618
    SpvExecutionModeRoundingModeRTPINTEL = 5620
    SpvExecutionModeRoundingModeRTNINTEL = 5621
    SpvExecutionModeFloatingPointModeALTINTEL = 5622
    SpvExecutionModeFloatingPointModeIEEEINTEL = 5623
    SpvExecutionModeMaxWorkgroupSizeINTEL = 5893
    SpvExecutionModeMaxWorkDimINTEL = 5894
    SpvExecutionModeNoGlobalOffsetINTEL = 5895
    SpvExecutionModeNumSIMDWorkitemsINTEL = 5896
    SpvExecutionModeSchedulerTargetFmaxMhzINTEL = 5903
    SpvExecutionModeNamedBarrierCountINTEL = 6417
    SpvExecutionModeMax = 2147483647
end

const SpvExecutionMode = SpvExecutionMode_

@cenum SpvStorageClass_::UInt32 begin
    SpvStorageClassUniformConstant = 0
    SpvStorageClassInput = 1
    SpvStorageClassUniform = 2
    SpvStorageClassOutput = 3
    SpvStorageClassWorkgroup = 4
    SpvStorageClassCrossWorkgroup = 5
    SpvStorageClassPrivate = 6
    SpvStorageClassFunction = 7
    SpvStorageClassGeneric = 8
    SpvStorageClassPushConstant = 9
    SpvStorageClassAtomicCounter = 10
    SpvStorageClassImage = 11
    SpvStorageClassStorageBuffer = 12
    SpvStorageClassCallableDataKHR = 5328
    SpvStorageClassCallableDataNV = 5328
    SpvStorageClassIncomingCallableDataKHR = 5329
    SpvStorageClassIncomingCallableDataNV = 5329
    SpvStorageClassRayPayloadKHR = 5338
    SpvStorageClassRayPayloadNV = 5338
    SpvStorageClassHitAttributeKHR = 5339
    SpvStorageClassHitAttributeNV = 5339
    SpvStorageClassIncomingRayPayloadKHR = 5342
    SpvStorageClassIncomingRayPayloadNV = 5342
    SpvStorageClassShaderRecordBufferKHR = 5343
    SpvStorageClassShaderRecordBufferNV = 5343
    SpvStorageClassPhysicalStorageBuffer = 5349
    SpvStorageClassPhysicalStorageBufferEXT = 5349
    SpvStorageClassCodeSectionINTEL = 5605
    SpvStorageClassDeviceOnlyINTEL = 5936
    SpvStorageClassHostOnlyINTEL = 5937
    SpvStorageClassMax = 2147483647
end

const SpvStorageClass = SpvStorageClass_

@cenum SpvDim_::UInt32 begin
    SpvDim1D = 0
    SpvDim2D = 1
    SpvDim3D = 2
    SpvDimCube = 3
    SpvDimRect = 4
    SpvDimBuffer = 5
    SpvDimSubpassData = 6
    SpvDimMax = 2147483647
end

const SpvDim = SpvDim_

@cenum SpvSamplerAddressingMode_::UInt32 begin
    SpvSamplerAddressingModeNone = 0
    SpvSamplerAddressingModeClampToEdge = 1
    SpvSamplerAddressingModeClamp = 2
    SpvSamplerAddressingModeRepeat = 3
    SpvSamplerAddressingModeRepeatMirrored = 4
    SpvSamplerAddressingModeMax = 2147483647
end

const SpvSamplerAddressingMode = SpvSamplerAddressingMode_

@cenum SpvSamplerFilterMode_::UInt32 begin
    SpvSamplerFilterModeNearest = 0
    SpvSamplerFilterModeLinear = 1
    SpvSamplerFilterModeMax = 2147483647
end

const SpvSamplerFilterMode = SpvSamplerFilterMode_

@cenum SpvImageFormat_::UInt32 begin
    SpvImageFormatUnknown = 0
    SpvImageFormatRgba32f = 1
    SpvImageFormatRgba16f = 2
    SpvImageFormatR32f = 3
    SpvImageFormatRgba8 = 4
    SpvImageFormatRgba8Snorm = 5
    SpvImageFormatRg32f = 6
    SpvImageFormatRg16f = 7
    SpvImageFormatR11fG11fB10f = 8
    SpvImageFormatR16f = 9
    SpvImageFormatRgba16 = 10
    SpvImageFormatRgb10A2 = 11
    SpvImageFormatRg16 = 12
    SpvImageFormatRg8 = 13
    SpvImageFormatR16 = 14
    SpvImageFormatR8 = 15
    SpvImageFormatRgba16Snorm = 16
    SpvImageFormatRg16Snorm = 17
    SpvImageFormatRg8Snorm = 18
    SpvImageFormatR16Snorm = 19
    SpvImageFormatR8Snorm = 20
    SpvImageFormatRgba32i = 21
    SpvImageFormatRgba16i = 22
    SpvImageFormatRgba8i = 23
    SpvImageFormatR32i = 24
    SpvImageFormatRg32i = 25
    SpvImageFormatRg16i = 26
    SpvImageFormatRg8i = 27
    SpvImageFormatR16i = 28
    SpvImageFormatR8i = 29
    SpvImageFormatRgba32ui = 30
    SpvImageFormatRgba16ui = 31
    SpvImageFormatRgba8ui = 32
    SpvImageFormatR32ui = 33
    SpvImageFormatRgb10a2ui = 34
    SpvImageFormatRg32ui = 35
    SpvImageFormatRg16ui = 36
    SpvImageFormatRg8ui = 37
    SpvImageFormatR16ui = 38
    SpvImageFormatR8ui = 39
    SpvImageFormatR64ui = 40
    SpvImageFormatR64i = 41
    SpvImageFormatMax = 2147483647
end

const SpvImageFormat = SpvImageFormat_

@cenum SpvImageChannelOrder_::UInt32 begin
    SpvImageChannelOrderR = 0
    SpvImageChannelOrderA = 1
    SpvImageChannelOrderRG = 2
    SpvImageChannelOrderRA = 3
    SpvImageChannelOrderRGB = 4
    SpvImageChannelOrderRGBA = 5
    SpvImageChannelOrderBGRA = 6
    SpvImageChannelOrderARGB = 7
    SpvImageChannelOrderIntensity = 8
    SpvImageChannelOrderLuminance = 9
    SpvImageChannelOrderRx = 10
    SpvImageChannelOrderRGx = 11
    SpvImageChannelOrderRGBx = 12
    SpvImageChannelOrderDepth = 13
    SpvImageChannelOrderDepthStencil = 14
    SpvImageChannelOrdersRGB = 15
    SpvImageChannelOrdersRGBx = 16
    SpvImageChannelOrdersRGBA = 17
    SpvImageChannelOrdersBGRA = 18
    SpvImageChannelOrderABGR = 19
    SpvImageChannelOrderMax = 2147483647
end

const SpvImageChannelOrder = SpvImageChannelOrder_

@cenum SpvImageChannelDataType_::UInt32 begin
    SpvImageChannelDataTypeSnormInt8 = 0
    SpvImageChannelDataTypeSnormInt16 = 1
    SpvImageChannelDataTypeUnormInt8 = 2
    SpvImageChannelDataTypeUnormInt16 = 3
    SpvImageChannelDataTypeUnormShort565 = 4
    SpvImageChannelDataTypeUnormShort555 = 5
    SpvImageChannelDataTypeUnormInt101010 = 6
    SpvImageChannelDataTypeSignedInt8 = 7
    SpvImageChannelDataTypeSignedInt16 = 8
    SpvImageChannelDataTypeSignedInt32 = 9
    SpvImageChannelDataTypeUnsignedInt8 = 10
    SpvImageChannelDataTypeUnsignedInt16 = 11
    SpvImageChannelDataTypeUnsignedInt32 = 12
    SpvImageChannelDataTypeHalfFloat = 13
    SpvImageChannelDataTypeFloat = 14
    SpvImageChannelDataTypeUnormInt24 = 15
    SpvImageChannelDataTypeUnormInt101010_2 = 16
    SpvImageChannelDataTypeMax = 2147483647
end

const SpvImageChannelDataType = SpvImageChannelDataType_

@cenum SpvImageOperandsShift_::UInt32 begin
    SpvImageOperandsBiasShift = 0
    SpvImageOperandsLodShift = 1
    SpvImageOperandsGradShift = 2
    SpvImageOperandsConstOffsetShift = 3
    SpvImageOperandsOffsetShift = 4
    SpvImageOperandsConstOffsetsShift = 5
    SpvImageOperandsSampleShift = 6
    SpvImageOperandsMinLodShift = 7
    SpvImageOperandsMakeTexelAvailableShift = 8
    SpvImageOperandsMakeTexelAvailableKHRShift = 8
    SpvImageOperandsMakeTexelVisibleShift = 9
    SpvImageOperandsMakeTexelVisibleKHRShift = 9
    SpvImageOperandsNonPrivateTexelShift = 10
    SpvImageOperandsNonPrivateTexelKHRShift = 10
    SpvImageOperandsVolatileTexelShift = 11
    SpvImageOperandsVolatileTexelKHRShift = 11
    SpvImageOperandsSignExtendShift = 12
    SpvImageOperandsZeroExtendShift = 13
    SpvImageOperandsNontemporalShift = 14
    SpvImageOperandsOffsetsShift = 16
    SpvImageOperandsMax = 2147483647
end

const SpvImageOperandsShift = SpvImageOperandsShift_

@cenum SpvImageOperandsMask_::UInt32 begin
    SpvImageOperandsMaskNone = 0
    SpvImageOperandsBiasMask = 1
    SpvImageOperandsLodMask = 2
    SpvImageOperandsGradMask = 4
    SpvImageOperandsConstOffsetMask = 8
    SpvImageOperandsOffsetMask = 16
    SpvImageOperandsConstOffsetsMask = 32
    SpvImageOperandsSampleMask = 64
    SpvImageOperandsMinLodMask = 128
    SpvImageOperandsMakeTexelAvailableMask = 256
    SpvImageOperandsMakeTexelAvailableKHRMask = 256
    SpvImageOperandsMakeTexelVisibleMask = 512
    SpvImageOperandsMakeTexelVisibleKHRMask = 512
    SpvImageOperandsNonPrivateTexelMask = 1024
    SpvImageOperandsNonPrivateTexelKHRMask = 1024
    SpvImageOperandsVolatileTexelMask = 2048
    SpvImageOperandsVolatileTexelKHRMask = 2048
    SpvImageOperandsSignExtendMask = 4096
    SpvImageOperandsZeroExtendMask = 8192
    SpvImageOperandsNontemporalMask = 16384
    SpvImageOperandsOffsetsMask = 65536
end

const SpvImageOperandsMask = SpvImageOperandsMask_

@cenum SpvFPFastMathModeShift_::UInt32 begin
    SpvFPFastMathModeNotNaNShift = 0
    SpvFPFastMathModeNotInfShift = 1
    SpvFPFastMathModeNSZShift = 2
    SpvFPFastMathModeAllowRecipShift = 3
    SpvFPFastMathModeFastShift = 4
    SpvFPFastMathModeAllowContractFastINTELShift = 16
    SpvFPFastMathModeAllowReassocINTELShift = 17
    SpvFPFastMathModeMax = 2147483647
end

const SpvFPFastMathModeShift = SpvFPFastMathModeShift_

@cenum SpvFPFastMathModeMask_::UInt32 begin
    SpvFPFastMathModeMaskNone = 0
    SpvFPFastMathModeNotNaNMask = 1
    SpvFPFastMathModeNotInfMask = 2
    SpvFPFastMathModeNSZMask = 4
    SpvFPFastMathModeAllowRecipMask = 8
    SpvFPFastMathModeFastMask = 16
    SpvFPFastMathModeAllowContractFastINTELMask = 65536
    SpvFPFastMathModeAllowReassocINTELMask = 131072
end

const SpvFPFastMathModeMask = SpvFPFastMathModeMask_

@cenum SpvFPRoundingMode_::UInt32 begin
    SpvFPRoundingModeRTE = 0
    SpvFPRoundingModeRTZ = 1
    SpvFPRoundingModeRTP = 2
    SpvFPRoundingModeRTN = 3
    SpvFPRoundingModeMax = 2147483647
end

const SpvFPRoundingMode = SpvFPRoundingMode_

@cenum SpvLinkageType_::UInt32 begin
    SpvLinkageTypeExport = 0
    SpvLinkageTypeImport = 1
    SpvLinkageTypeLinkOnceODR = 2
    SpvLinkageTypeMax = 2147483647
end

const SpvLinkageType = SpvLinkageType_

@cenum SpvAccessQualifier_::UInt32 begin
    SpvAccessQualifierReadOnly = 0
    SpvAccessQualifierWriteOnly = 1
    SpvAccessQualifierReadWrite = 2
    SpvAccessQualifierMax = 2147483647
end

const SpvAccessQualifier = SpvAccessQualifier_

@cenum SpvFunctionParameterAttribute_::UInt32 begin
    SpvFunctionParameterAttributeZext = 0
    SpvFunctionParameterAttributeSext = 1
    SpvFunctionParameterAttributeByVal = 2
    SpvFunctionParameterAttributeSret = 3
    SpvFunctionParameterAttributeNoAlias = 4
    SpvFunctionParameterAttributeNoCapture = 5
    SpvFunctionParameterAttributeNoWrite = 6
    SpvFunctionParameterAttributeNoReadWrite = 7
    SpvFunctionParameterAttributeMax = 2147483647
end

const SpvFunctionParameterAttribute = SpvFunctionParameterAttribute_

@cenum SpvDecoration_::UInt32 begin
    SpvDecorationRelaxedPrecision = 0
    SpvDecorationSpecId = 1
    SpvDecorationBlock = 2
    SpvDecorationBufferBlock = 3
    SpvDecorationRowMajor = 4
    SpvDecorationColMajor = 5
    SpvDecorationArrayStride = 6
    SpvDecorationMatrixStride = 7
    SpvDecorationGLSLShared = 8
    SpvDecorationGLSLPacked = 9
    SpvDecorationCPacked = 10
    SpvDecorationBuiltIn = 11
    SpvDecorationNoPerspective = 13
    SpvDecorationFlat = 14
    SpvDecorationPatch = 15
    SpvDecorationCentroid = 16
    SpvDecorationSample = 17
    SpvDecorationInvariant = 18
    SpvDecorationRestrict = 19
    SpvDecorationAliased = 20
    SpvDecorationVolatile = 21
    SpvDecorationConstant = 22
    SpvDecorationCoherent = 23
    SpvDecorationNonWritable = 24
    SpvDecorationNonReadable = 25
    SpvDecorationUniform = 26
    SpvDecorationUniformId = 27
    SpvDecorationSaturatedConversion = 28
    SpvDecorationStream = 29
    SpvDecorationLocation = 30
    SpvDecorationComponent = 31
    SpvDecorationIndex = 32
    SpvDecorationBinding = 33
    SpvDecorationDescriptorSet = 34
    SpvDecorationOffset = 35
    SpvDecorationXfbBuffer = 36
    SpvDecorationXfbStride = 37
    SpvDecorationFuncParamAttr = 38
    SpvDecorationFPRoundingMode = 39
    SpvDecorationFPFastMathMode = 40
    SpvDecorationLinkageAttributes = 41
    SpvDecorationNoContraction = 42
    SpvDecorationInputAttachmentIndex = 43
    SpvDecorationAlignment = 44
    SpvDecorationMaxByteOffset = 45
    SpvDecorationAlignmentId = 46
    SpvDecorationMaxByteOffsetId = 47
    SpvDecorationNoSignedWrap = 4469
    SpvDecorationNoUnsignedWrap = 4470
    SpvDecorationExplicitInterpAMD = 4999
    SpvDecorationOverrideCoverageNV = 5248
    SpvDecorationPassthroughNV = 5250
    SpvDecorationViewportRelativeNV = 5252
    SpvDecorationSecondaryViewportRelativeNV = 5256
    SpvDecorationPerPrimitiveNV = 5271
    SpvDecorationPerViewNV = 5272
    SpvDecorationPerTaskNV = 5273
    SpvDecorationPerVertexKHR = 5285
    SpvDecorationPerVertexNV = 5285
    SpvDecorationNonUniform = 5300
    SpvDecorationNonUniformEXT = 5300
    SpvDecorationRestrictPointer = 5355
    SpvDecorationRestrictPointerEXT = 5355
    SpvDecorationAliasedPointer = 5356
    SpvDecorationAliasedPointerEXT = 5356
    SpvDecorationBindlessSamplerNV = 5398
    SpvDecorationBindlessImageNV = 5399
    SpvDecorationBoundSamplerNV = 5400
    SpvDecorationBoundImageNV = 5401
    SpvDecorationSIMTCallINTEL = 5599
    SpvDecorationReferencedIndirectlyINTEL = 5602
    SpvDecorationClobberINTEL = 5607
    SpvDecorationSideEffectsINTEL = 5608
    SpvDecorationVectorComputeVariableINTEL = 5624
    SpvDecorationFuncParamIOKindINTEL = 5625
    SpvDecorationVectorComputeFunctionINTEL = 5626
    SpvDecorationStackCallINTEL = 5627
    SpvDecorationGlobalVariableOffsetINTEL = 5628
    SpvDecorationCounterBuffer = 5634
    SpvDecorationHlslCounterBufferGOOGLE = 5634
    SpvDecorationHlslSemanticGOOGLE = 5635
    SpvDecorationUserSemantic = 5635
    SpvDecorationUserTypeGOOGLE = 5636
    SpvDecorationFunctionRoundingModeINTEL = 5822
    SpvDecorationFunctionDenormModeINTEL = 5823
    SpvDecorationRegisterINTEL = 5825
    SpvDecorationMemoryINTEL = 5826
    SpvDecorationNumbanksINTEL = 5827
    SpvDecorationBankwidthINTEL = 5828
    SpvDecorationMaxPrivateCopiesINTEL = 5829
    SpvDecorationSinglepumpINTEL = 5830
    SpvDecorationDoublepumpINTEL = 5831
    SpvDecorationMaxReplicatesINTEL = 5832
    SpvDecorationSimpleDualPortINTEL = 5833
    SpvDecorationMergeINTEL = 5834
    SpvDecorationBankBitsINTEL = 5835
    SpvDecorationForcePow2DepthINTEL = 5836
    SpvDecorationBurstCoalesceINTEL = 5899
    SpvDecorationCacheSizeINTEL = 5900
    SpvDecorationDontStaticallyCoalesceINTEL = 5901
    SpvDecorationPrefetchINTEL = 5902
    SpvDecorationStallEnableINTEL = 5905
    SpvDecorationFuseLoopsInFunctionINTEL = 5907
    SpvDecorationAliasScopeINTEL = 5914
    SpvDecorationNoAliasINTEL = 5915
    SpvDecorationBufferLocationINTEL = 5921
    SpvDecorationIOPipeStorageINTEL = 5944
    SpvDecorationFunctionFloatingPointModeINTEL = 6080
    SpvDecorationSingleElementVectorINTEL = 6085
    SpvDecorationVectorComputeCallableFunctionINTEL = 6087
    SpvDecorationMediaBlockIOINTEL = 6140
    SpvDecorationMax = 2147483647
end

const SpvDecoration = SpvDecoration_

@cenum SpvBuiltIn_::UInt32 begin
    SpvBuiltInPosition = 0
    SpvBuiltInPointSize = 1
    SpvBuiltInClipDistance = 3
    SpvBuiltInCullDistance = 4
    SpvBuiltInVertexId = 5
    SpvBuiltInInstanceId = 6
    SpvBuiltInPrimitiveId = 7
    SpvBuiltInInvocationId = 8
    SpvBuiltInLayer = 9
    SpvBuiltInViewportIndex = 10
    SpvBuiltInTessLevelOuter = 11
    SpvBuiltInTessLevelInner = 12
    SpvBuiltInTessCoord = 13
    SpvBuiltInPatchVertices = 14
    SpvBuiltInFragCoord = 15
    SpvBuiltInPointCoord = 16
    SpvBuiltInFrontFacing = 17
    SpvBuiltInSampleId = 18
    SpvBuiltInSamplePosition = 19
    SpvBuiltInSampleMask = 20
    SpvBuiltInFragDepth = 22
    SpvBuiltInHelperInvocation = 23
    SpvBuiltInNumWorkgroups = 24
    SpvBuiltInWorkgroupSize = 25
    SpvBuiltInWorkgroupId = 26
    SpvBuiltInLocalInvocationId = 27
    SpvBuiltInGlobalInvocationId = 28
    SpvBuiltInLocalInvocationIndex = 29
    SpvBuiltInWorkDim = 30
    SpvBuiltInGlobalSize = 31
    SpvBuiltInEnqueuedWorkgroupSize = 32
    SpvBuiltInGlobalOffset = 33
    SpvBuiltInGlobalLinearId = 34
    SpvBuiltInSubgroupSize = 36
    SpvBuiltInSubgroupMaxSize = 37
    SpvBuiltInNumSubgroups = 38
    SpvBuiltInNumEnqueuedSubgroups = 39
    SpvBuiltInSubgroupId = 40
    SpvBuiltInSubgroupLocalInvocationId = 41
    SpvBuiltInVertexIndex = 42
    SpvBuiltInInstanceIndex = 43
    SpvBuiltInSubgroupEqMask = 4416
    SpvBuiltInSubgroupEqMaskKHR = 4416
    SpvBuiltInSubgroupGeMask = 4417
    SpvBuiltInSubgroupGeMaskKHR = 4417
    SpvBuiltInSubgroupGtMask = 4418
    SpvBuiltInSubgroupGtMaskKHR = 4418
    SpvBuiltInSubgroupLeMask = 4419
    SpvBuiltInSubgroupLeMaskKHR = 4419
    SpvBuiltInSubgroupLtMask = 4420
    SpvBuiltInSubgroupLtMaskKHR = 4420
    SpvBuiltInBaseVertex = 4424
    SpvBuiltInBaseInstance = 4425
    SpvBuiltInDrawIndex = 4426
    SpvBuiltInPrimitiveShadingRateKHR = 4432
    SpvBuiltInDeviceIndex = 4438
    SpvBuiltInViewIndex = 4440
    SpvBuiltInShadingRateKHR = 4444
    SpvBuiltInBaryCoordNoPerspAMD = 4992
    SpvBuiltInBaryCoordNoPerspCentroidAMD = 4993
    SpvBuiltInBaryCoordNoPerspSampleAMD = 4994
    SpvBuiltInBaryCoordSmoothAMD = 4995
    SpvBuiltInBaryCoordSmoothCentroidAMD = 4996
    SpvBuiltInBaryCoordSmoothSampleAMD = 4997
    SpvBuiltInBaryCoordPullModelAMD = 4998
    SpvBuiltInFragStencilRefEXT = 5014
    SpvBuiltInViewportMaskNV = 5253
    SpvBuiltInSecondaryPositionNV = 5257
    SpvBuiltInSecondaryViewportMaskNV = 5258
    SpvBuiltInPositionPerViewNV = 5261
    SpvBuiltInViewportMaskPerViewNV = 5262
    SpvBuiltInFullyCoveredEXT = 5264
    SpvBuiltInTaskCountNV = 5274
    SpvBuiltInPrimitiveCountNV = 5275
    SpvBuiltInPrimitiveIndicesNV = 5276
    SpvBuiltInClipDistancePerViewNV = 5277
    SpvBuiltInCullDistancePerViewNV = 5278
    SpvBuiltInLayerPerViewNV = 5279
    SpvBuiltInMeshViewCountNV = 5280
    SpvBuiltInMeshViewIndicesNV = 5281
    SpvBuiltInBaryCoordKHR = 5286
    SpvBuiltInBaryCoordNV = 5286
    SpvBuiltInBaryCoordNoPerspKHR = 5287
    SpvBuiltInBaryCoordNoPerspNV = 5287
    SpvBuiltInFragSizeEXT = 5292
    SpvBuiltInFragmentSizeNV = 5292
    SpvBuiltInFragInvocationCountEXT = 5293
    SpvBuiltInInvocationsPerPixelNV = 5293
    SpvBuiltInLaunchIdKHR = 5319
    SpvBuiltInLaunchIdNV = 5319
    SpvBuiltInLaunchSizeKHR = 5320
    SpvBuiltInLaunchSizeNV = 5320
    SpvBuiltInWorldRayOriginKHR = 5321
    SpvBuiltInWorldRayOriginNV = 5321
    SpvBuiltInWorldRayDirectionKHR = 5322
    SpvBuiltInWorldRayDirectionNV = 5322
    SpvBuiltInObjectRayOriginKHR = 5323
    SpvBuiltInObjectRayOriginNV = 5323
    SpvBuiltInObjectRayDirectionKHR = 5324
    SpvBuiltInObjectRayDirectionNV = 5324
    SpvBuiltInRayTminKHR = 5325
    SpvBuiltInRayTminNV = 5325
    SpvBuiltInRayTmaxKHR = 5326
    SpvBuiltInRayTmaxNV = 5326
    SpvBuiltInInstanceCustomIndexKHR = 5327
    SpvBuiltInInstanceCustomIndexNV = 5327
    SpvBuiltInObjectToWorldKHR = 5330
    SpvBuiltInObjectToWorldNV = 5330
    SpvBuiltInWorldToObjectKHR = 5331
    SpvBuiltInWorldToObjectNV = 5331
    SpvBuiltInHitTNV = 5332
    SpvBuiltInHitKindKHR = 5333
    SpvBuiltInHitKindNV = 5333
    SpvBuiltInCurrentRayTimeNV = 5334
    SpvBuiltInIncomingRayFlagsKHR = 5351
    SpvBuiltInIncomingRayFlagsNV = 5351
    SpvBuiltInRayGeometryIndexKHR = 5352
    SpvBuiltInWarpsPerSMNV = 5374
    SpvBuiltInSMCountNV = 5375
    SpvBuiltInWarpIDNV = 5376
    SpvBuiltInSMIDNV = 5377
    SpvBuiltInCullMaskKHR = 6021
    SpvBuiltInMax = 2147483647
end

const SpvBuiltIn = SpvBuiltIn_

@cenum SpvSelectionControlShift_::UInt32 begin
    SpvSelectionControlFlattenShift = 0
    SpvSelectionControlDontFlattenShift = 1
    SpvSelectionControlMax = 2147483647
end

const SpvSelectionControlShift = SpvSelectionControlShift_

@cenum SpvSelectionControlMask_::UInt32 begin
    SpvSelectionControlMaskNone = 0
    SpvSelectionControlFlattenMask = 1
    SpvSelectionControlDontFlattenMask = 2
end

const SpvSelectionControlMask = SpvSelectionControlMask_

@cenum SpvLoopControlShift_::UInt32 begin
    SpvLoopControlUnrollShift = 0
    SpvLoopControlDontUnrollShift = 1
    SpvLoopControlDependencyInfiniteShift = 2
    SpvLoopControlDependencyLengthShift = 3
    SpvLoopControlMinIterationsShift = 4
    SpvLoopControlMaxIterationsShift = 5
    SpvLoopControlIterationMultipleShift = 6
    SpvLoopControlPeelCountShift = 7
    SpvLoopControlPartialCountShift = 8
    SpvLoopControlInitiationIntervalINTELShift = 16
    SpvLoopControlMaxConcurrencyINTELShift = 17
    SpvLoopControlDependencyArrayINTELShift = 18
    SpvLoopControlPipelineEnableINTELShift = 19
    SpvLoopControlLoopCoalesceINTELShift = 20
    SpvLoopControlMaxInterleavingINTELShift = 21
    SpvLoopControlSpeculatedIterationsINTELShift = 22
    SpvLoopControlNoFusionINTELShift = 23
    SpvLoopControlMax = 2147483647
end

const SpvLoopControlShift = SpvLoopControlShift_

@cenum SpvLoopControlMask_::UInt32 begin
    SpvLoopControlMaskNone = 0
    SpvLoopControlUnrollMask = 1
    SpvLoopControlDontUnrollMask = 2
    SpvLoopControlDependencyInfiniteMask = 4
    SpvLoopControlDependencyLengthMask = 8
    SpvLoopControlMinIterationsMask = 16
    SpvLoopControlMaxIterationsMask = 32
    SpvLoopControlIterationMultipleMask = 64
    SpvLoopControlPeelCountMask = 128
    SpvLoopControlPartialCountMask = 256
    SpvLoopControlInitiationIntervalINTELMask = 65536
    SpvLoopControlMaxConcurrencyINTELMask = 131072
    SpvLoopControlDependencyArrayINTELMask = 262144
    SpvLoopControlPipelineEnableINTELMask = 524288
    SpvLoopControlLoopCoalesceINTELMask = 1048576
    SpvLoopControlMaxInterleavingINTELMask = 2097152
    SpvLoopControlSpeculatedIterationsINTELMask = 4194304
    SpvLoopControlNoFusionINTELMask = 8388608
end

const SpvLoopControlMask = SpvLoopControlMask_

@cenum SpvFunctionControlShift_::UInt32 begin
    SpvFunctionControlInlineShift = 0
    SpvFunctionControlDontInlineShift = 1
    SpvFunctionControlPureShift = 2
    SpvFunctionControlConstShift = 3
    SpvFunctionControlOptNoneINTELShift = 16
    SpvFunctionControlMax = 2147483647
end

const SpvFunctionControlShift = SpvFunctionControlShift_

@cenum SpvFunctionControlMask_::UInt32 begin
    SpvFunctionControlMaskNone = 0
    SpvFunctionControlInlineMask = 1
    SpvFunctionControlDontInlineMask = 2
    SpvFunctionControlPureMask = 4
    SpvFunctionControlConstMask = 8
    SpvFunctionControlOptNoneINTELMask = 65536
end

const SpvFunctionControlMask = SpvFunctionControlMask_

@cenum SpvMemorySemanticsShift_::UInt32 begin
    SpvMemorySemanticsAcquireShift = 1
    SpvMemorySemanticsReleaseShift = 2
    SpvMemorySemanticsAcquireReleaseShift = 3
    SpvMemorySemanticsSequentiallyConsistentShift = 4
    SpvMemorySemanticsUniformMemoryShift = 6
    SpvMemorySemanticsSubgroupMemoryShift = 7
    SpvMemorySemanticsWorkgroupMemoryShift = 8
    SpvMemorySemanticsCrossWorkgroupMemoryShift = 9
    SpvMemorySemanticsAtomicCounterMemoryShift = 10
    SpvMemorySemanticsImageMemoryShift = 11
    SpvMemorySemanticsOutputMemoryShift = 12
    SpvMemorySemanticsOutputMemoryKHRShift = 12
    SpvMemorySemanticsMakeAvailableShift = 13
    SpvMemorySemanticsMakeAvailableKHRShift = 13
    SpvMemorySemanticsMakeVisibleShift = 14
    SpvMemorySemanticsMakeVisibleKHRShift = 14
    SpvMemorySemanticsVolatileShift = 15
    SpvMemorySemanticsMax = 2147483647
end

const SpvMemorySemanticsShift = SpvMemorySemanticsShift_

@cenum SpvMemorySemanticsMask_::UInt32 begin
    SpvMemorySemanticsMaskNone = 0
    SpvMemorySemanticsAcquireMask = 2
    SpvMemorySemanticsReleaseMask = 4
    SpvMemorySemanticsAcquireReleaseMask = 8
    SpvMemorySemanticsSequentiallyConsistentMask = 16
    SpvMemorySemanticsUniformMemoryMask = 64
    SpvMemorySemanticsSubgroupMemoryMask = 128
    SpvMemorySemanticsWorkgroupMemoryMask = 256
    SpvMemorySemanticsCrossWorkgroupMemoryMask = 512
    SpvMemorySemanticsAtomicCounterMemoryMask = 1024
    SpvMemorySemanticsImageMemoryMask = 2048
    SpvMemorySemanticsOutputMemoryMask = 4096
    SpvMemorySemanticsOutputMemoryKHRMask = 4096
    SpvMemorySemanticsMakeAvailableMask = 8192
    SpvMemorySemanticsMakeAvailableKHRMask = 8192
    SpvMemorySemanticsMakeVisibleMask = 16384
    SpvMemorySemanticsMakeVisibleKHRMask = 16384
    SpvMemorySemanticsVolatileMask = 32768
end

const SpvMemorySemanticsMask = SpvMemorySemanticsMask_

@cenum SpvMemoryAccessShift_::UInt32 begin
    SpvMemoryAccessVolatileShift = 0
    SpvMemoryAccessAlignedShift = 1
    SpvMemoryAccessNontemporalShift = 2
    SpvMemoryAccessMakePointerAvailableShift = 3
    SpvMemoryAccessMakePointerAvailableKHRShift = 3
    SpvMemoryAccessMakePointerVisibleShift = 4
    SpvMemoryAccessMakePointerVisibleKHRShift = 4
    SpvMemoryAccessNonPrivatePointerShift = 5
    SpvMemoryAccessNonPrivatePointerKHRShift = 5
    SpvMemoryAccessAliasScopeINTELMaskShift = 16
    SpvMemoryAccessNoAliasINTELMaskShift = 17
    SpvMemoryAccessMax = 2147483647
end

const SpvMemoryAccessShift = SpvMemoryAccessShift_

@cenum SpvMemoryAccessMask_::UInt32 begin
    SpvMemoryAccessMaskNone = 0
    SpvMemoryAccessVolatileMask = 1
    SpvMemoryAccessAlignedMask = 2
    SpvMemoryAccessNontemporalMask = 4
    SpvMemoryAccessMakePointerAvailableMask = 8
    SpvMemoryAccessMakePointerAvailableKHRMask = 8
    SpvMemoryAccessMakePointerVisibleMask = 16
    SpvMemoryAccessMakePointerVisibleKHRMask = 16
    SpvMemoryAccessNonPrivatePointerMask = 32
    SpvMemoryAccessNonPrivatePointerKHRMask = 32
    SpvMemoryAccessAliasScopeINTELMaskMask = 65536
    SpvMemoryAccessNoAliasINTELMaskMask = 131072
end

const SpvMemoryAccessMask = SpvMemoryAccessMask_

@cenum SpvScope_::UInt32 begin
    SpvScopeCrossDevice = 0
    SpvScopeDevice = 1
    SpvScopeWorkgroup = 2
    SpvScopeSubgroup = 3
    SpvScopeInvocation = 4
    SpvScopeQueueFamily = 5
    SpvScopeQueueFamilyKHR = 5
    SpvScopeShaderCallKHR = 6
    SpvScopeMax = 2147483647
end

const SpvScope = SpvScope_

@cenum SpvGroupOperation_::UInt32 begin
    SpvGroupOperationReduce = 0
    SpvGroupOperationInclusiveScan = 1
    SpvGroupOperationExclusiveScan = 2
    SpvGroupOperationClusteredReduce = 3
    SpvGroupOperationPartitionedReduceNV = 6
    SpvGroupOperationPartitionedInclusiveScanNV = 7
    SpvGroupOperationPartitionedExclusiveScanNV = 8
    SpvGroupOperationMax = 2147483647
end

const SpvGroupOperation = SpvGroupOperation_

@cenum SpvKernelEnqueueFlags_::UInt32 begin
    SpvKernelEnqueueFlagsNoWait = 0
    SpvKernelEnqueueFlagsWaitKernel = 1
    SpvKernelEnqueueFlagsWaitWorkGroup = 2
    SpvKernelEnqueueFlagsMax = 2147483647
end

const SpvKernelEnqueueFlags = SpvKernelEnqueueFlags_

@cenum SpvKernelProfilingInfoShift_::UInt32 begin
    SpvKernelProfilingInfoCmdExecTimeShift = 0
    SpvKernelProfilingInfoMax = 2147483647
end

const SpvKernelProfilingInfoShift = SpvKernelProfilingInfoShift_

@cenum SpvKernelProfilingInfoMask_::UInt32 begin
    SpvKernelProfilingInfoMaskNone = 0
    SpvKernelProfilingInfoCmdExecTimeMask = 1
end

const SpvKernelProfilingInfoMask = SpvKernelProfilingInfoMask_

@cenum SpvCapability_::UInt32 begin
    SpvCapabilityMatrix = 0
    SpvCapabilityShader = 1
    SpvCapabilityGeometry = 2
    SpvCapabilityTessellation = 3
    SpvCapabilityAddresses = 4
    SpvCapabilityLinkage = 5
    SpvCapabilityKernel = 6
    SpvCapabilityVector16 = 7
    SpvCapabilityFloat16Buffer = 8
    SpvCapabilityFloat16 = 9
    SpvCapabilityFloat64 = 10
    SpvCapabilityInt64 = 11
    SpvCapabilityInt64Atomics = 12
    SpvCapabilityImageBasic = 13
    SpvCapabilityImageReadWrite = 14
    SpvCapabilityImageMipmap = 15
    SpvCapabilityPipes = 17
    SpvCapabilityGroups = 18
    SpvCapabilityDeviceEnqueue = 19
    SpvCapabilityLiteralSampler = 20
    SpvCapabilityAtomicStorage = 21
    SpvCapabilityInt16 = 22
    SpvCapabilityTessellationPointSize = 23
    SpvCapabilityGeometryPointSize = 24
    SpvCapabilityImageGatherExtended = 25
    SpvCapabilityStorageImageMultisample = 27
    SpvCapabilityUniformBufferArrayDynamicIndexing = 28
    SpvCapabilitySampledImageArrayDynamicIndexing = 29
    SpvCapabilityStorageBufferArrayDynamicIndexing = 30
    SpvCapabilityStorageImageArrayDynamicIndexing = 31
    SpvCapabilityClipDistance = 32
    SpvCapabilityCullDistance = 33
    SpvCapabilityImageCubeArray = 34
    SpvCapabilitySampleRateShading = 35
    SpvCapabilityImageRect = 36
    SpvCapabilitySampledRect = 37
    SpvCapabilityGenericPointer = 38
    SpvCapabilityInt8 = 39
    SpvCapabilityInputAttachment = 40
    SpvCapabilitySparseResidency = 41
    SpvCapabilityMinLod = 42
    SpvCapabilitySampled1D = 43
    SpvCapabilityImage1D = 44
    SpvCapabilitySampledCubeArray = 45
    SpvCapabilitySampledBuffer = 46
    SpvCapabilityImageBuffer = 47
    SpvCapabilityImageMSArray = 48
    SpvCapabilityStorageImageExtendedFormats = 49
    SpvCapabilityImageQuery = 50
    SpvCapabilityDerivativeControl = 51
    SpvCapabilityInterpolationFunction = 52
    SpvCapabilityTransformFeedback = 53
    SpvCapabilityGeometryStreams = 54
    SpvCapabilityStorageImageReadWithoutFormat = 55
    SpvCapabilityStorageImageWriteWithoutFormat = 56
    SpvCapabilityMultiViewport = 57
    SpvCapabilitySubgroupDispatch = 58
    SpvCapabilityNamedBarrier = 59
    SpvCapabilityPipeStorage = 60
    SpvCapabilityGroupNonUniform = 61
    SpvCapabilityGroupNonUniformVote = 62
    SpvCapabilityGroupNonUniformArithmetic = 63
    SpvCapabilityGroupNonUniformBallot = 64
    SpvCapabilityGroupNonUniformShuffle = 65
    SpvCapabilityGroupNonUniformShuffleRelative = 66
    SpvCapabilityGroupNonUniformClustered = 67
    SpvCapabilityGroupNonUniformQuad = 68
    SpvCapabilityShaderLayer = 69
    SpvCapabilityShaderViewportIndex = 70
    SpvCapabilityUniformDecoration = 71
    SpvCapabilityFragmentShadingRateKHR = 4422
    SpvCapabilitySubgroupBallotKHR = 4423
    SpvCapabilityDrawParameters = 4427
    SpvCapabilityWorkgroupMemoryExplicitLayoutKHR = 4428
    SpvCapabilityWorkgroupMemoryExplicitLayout8BitAccessKHR = 4429
    SpvCapabilityWorkgroupMemoryExplicitLayout16BitAccessKHR = 4430
    SpvCapabilitySubgroupVoteKHR = 4431
    SpvCapabilityStorageBuffer16BitAccess = 4433
    SpvCapabilityStorageUniformBufferBlock16 = 4433
    SpvCapabilityStorageUniform16 = 4434
    SpvCapabilityUniformAndStorageBuffer16BitAccess = 4434
    SpvCapabilityStoragePushConstant16 = 4435
    SpvCapabilityStorageInputOutput16 = 4436
    SpvCapabilityDeviceGroup = 4437
    SpvCapabilityMultiView = 4439
    SpvCapabilityVariablePointersStorageBuffer = 4441
    SpvCapabilityVariablePointers = 4442
    SpvCapabilityAtomicStorageOps = 4445
    SpvCapabilitySampleMaskPostDepthCoverage = 4447
    SpvCapabilityStorageBuffer8BitAccess = 4448
    SpvCapabilityUniformAndStorageBuffer8BitAccess = 4449
    SpvCapabilityStoragePushConstant8 = 4450
    SpvCapabilityDenormPreserve = 4464
    SpvCapabilityDenormFlushToZero = 4465
    SpvCapabilitySignedZeroInfNanPreserve = 4466
    SpvCapabilityRoundingModeRTE = 4467
    SpvCapabilityRoundingModeRTZ = 4468
    SpvCapabilityRayQueryProvisionalKHR = 4471
    SpvCapabilityRayQueryKHR = 4472
    SpvCapabilityRayTraversalPrimitiveCullingKHR = 4478
    SpvCapabilityRayTracingKHR = 4479
    SpvCapabilityFloat16ImageAMD = 5008
    SpvCapabilityImageGatherBiasLodAMD = 5009
    SpvCapabilityFragmentMaskAMD = 5010
    SpvCapabilityStencilExportEXT = 5013
    SpvCapabilityImageReadWriteLodAMD = 5015
    SpvCapabilityInt64ImageEXT = 5016
    SpvCapabilityShaderClockKHR = 5055
    SpvCapabilitySampleMaskOverrideCoverageNV = 5249
    SpvCapabilityGeometryShaderPassthroughNV = 5251
    SpvCapabilityShaderViewportIndexLayerEXT = 5254
    SpvCapabilityShaderViewportIndexLayerNV = 5254
    SpvCapabilityShaderViewportMaskNV = 5255
    SpvCapabilityShaderStereoViewNV = 5259
    SpvCapabilityPerViewAttributesNV = 5260
    SpvCapabilityFragmentFullyCoveredEXT = 5265
    SpvCapabilityMeshShadingNV = 5266
    SpvCapabilityImageFootprintNV = 5282
    SpvCapabilityFragmentBarycentricKHR = 5284
    SpvCapabilityFragmentBarycentricNV = 5284
    SpvCapabilityComputeDerivativeGroupQuadsNV = 5288
    SpvCapabilityFragmentDensityEXT = 5291
    SpvCapabilityShadingRateNV = 5291
    SpvCapabilityGroupNonUniformPartitionedNV = 5297
    SpvCapabilityShaderNonUniform = 5301
    SpvCapabilityShaderNonUniformEXT = 5301
    SpvCapabilityRuntimeDescriptorArray = 5302
    SpvCapabilityRuntimeDescriptorArrayEXT = 5302
    SpvCapabilityInputAttachmentArrayDynamicIndexing = 5303
    SpvCapabilityInputAttachmentArrayDynamicIndexingEXT = 5303
    SpvCapabilityUniformTexelBufferArrayDynamicIndexing = 5304
    SpvCapabilityUniformTexelBufferArrayDynamicIndexingEXT = 5304
    SpvCapabilityStorageTexelBufferArrayDynamicIndexing = 5305
    SpvCapabilityStorageTexelBufferArrayDynamicIndexingEXT = 5305
    SpvCapabilityUniformBufferArrayNonUniformIndexing = 5306
    SpvCapabilityUniformBufferArrayNonUniformIndexingEXT = 5306
    SpvCapabilitySampledImageArrayNonUniformIndexing = 5307
    SpvCapabilitySampledImageArrayNonUniformIndexingEXT = 5307
    SpvCapabilityStorageBufferArrayNonUniformIndexing = 5308
    SpvCapabilityStorageBufferArrayNonUniformIndexingEXT = 5308
    SpvCapabilityStorageImageArrayNonUniformIndexing = 5309
    SpvCapabilityStorageImageArrayNonUniformIndexingEXT = 5309
    SpvCapabilityInputAttachmentArrayNonUniformIndexing = 5310
    SpvCapabilityInputAttachmentArrayNonUniformIndexingEXT = 5310
    SpvCapabilityUniformTexelBufferArrayNonUniformIndexing = 5311
    SpvCapabilityUniformTexelBufferArrayNonUniformIndexingEXT = 5311
    SpvCapabilityStorageTexelBufferArrayNonUniformIndexing = 5312
    SpvCapabilityStorageTexelBufferArrayNonUniformIndexingEXT = 5312
    SpvCapabilityRayTracingNV = 5340
    SpvCapabilityRayTracingMotionBlurNV = 5341
    SpvCapabilityVulkanMemoryModel = 5345
    SpvCapabilityVulkanMemoryModelKHR = 5345
    SpvCapabilityVulkanMemoryModelDeviceScope = 5346
    SpvCapabilityVulkanMemoryModelDeviceScopeKHR = 5346
    SpvCapabilityPhysicalStorageBufferAddresses = 5347
    SpvCapabilityPhysicalStorageBufferAddressesEXT = 5347
    SpvCapabilityComputeDerivativeGroupLinearNV = 5350
    SpvCapabilityRayTracingProvisionalKHR = 5353
    SpvCapabilityCooperativeMatrixNV = 5357
    SpvCapabilityFragmentShaderSampleInterlockEXT = 5363
    SpvCapabilityFragmentShaderShadingRateInterlockEXT = 5372
    SpvCapabilityShaderSMBuiltinsNV = 5373
    SpvCapabilityFragmentShaderPixelInterlockEXT = 5378
    SpvCapabilityDemoteToHelperInvocation = 5379
    SpvCapabilityDemoteToHelperInvocationEXT = 5379
    SpvCapabilityBindlessTextureNV = 5390
    SpvCapabilitySubgroupShuffleINTEL = 5568
    SpvCapabilitySubgroupBufferBlockIOINTEL = 5569
    SpvCapabilitySubgroupImageBlockIOINTEL = 5570
    SpvCapabilitySubgroupImageMediaBlockIOINTEL = 5579
    SpvCapabilityRoundToInfinityINTEL = 5582
    SpvCapabilityFloatingPointModeINTEL = 5583
    SpvCapabilityIntegerFunctions2INTEL = 5584
    SpvCapabilityFunctionPointersINTEL = 5603
    SpvCapabilityIndirectReferencesINTEL = 5604
    SpvCapabilityAsmINTEL = 5606
    SpvCapabilityAtomicFloat32MinMaxEXT = 5612
    SpvCapabilityAtomicFloat64MinMaxEXT = 5613
    SpvCapabilityAtomicFloat16MinMaxEXT = 5616
    SpvCapabilityVectorComputeINTEL = 5617
    SpvCapabilityVectorAnyINTEL = 5619
    SpvCapabilityExpectAssumeKHR = 5629
    SpvCapabilitySubgroupAvcMotionEstimationINTEL = 5696
    SpvCapabilitySubgroupAvcMotionEstimationIntraINTEL = 5697
    SpvCapabilitySubgroupAvcMotionEstimationChromaINTEL = 5698
    SpvCapabilityVariableLengthArrayINTEL = 5817
    SpvCapabilityFunctionFloatControlINTEL = 5821
    SpvCapabilityFPGAMemoryAttributesINTEL = 5824
    SpvCapabilityFPFastMathModeINTEL = 5837
    SpvCapabilityArbitraryPrecisionIntegersINTEL = 5844
    SpvCapabilityArbitraryPrecisionFloatingPointINTEL = 5845
    SpvCapabilityUnstructuredLoopControlsINTEL = 5886
    SpvCapabilityFPGALoopControlsINTEL = 5888
    SpvCapabilityKernelAttributesINTEL = 5892
    SpvCapabilityFPGAKernelAttributesINTEL = 5897
    SpvCapabilityFPGAMemoryAccessesINTEL = 5898
    SpvCapabilityFPGAClusterAttributesINTEL = 5904
    SpvCapabilityLoopFuseINTEL = 5906
    SpvCapabilityMemoryAccessAliasingINTEL = 5910
    SpvCapabilityFPGABufferLocationINTEL = 5920
    SpvCapabilityArbitraryPrecisionFixedPointINTEL = 5922
    SpvCapabilityUSMStorageClassesINTEL = 5935
    SpvCapabilityIOPipesINTEL = 5943
    SpvCapabilityBlockingPipesINTEL = 5945
    SpvCapabilityFPGARegINTEL = 5948
    SpvCapabilityDotProductInputAll = 6016
    SpvCapabilityDotProductInputAllKHR = 6016
    SpvCapabilityDotProductInput4x8Bit = 6017
    SpvCapabilityDotProductInput4x8BitKHR = 6017
    SpvCapabilityDotProductInput4x8BitPacked = 6018
    SpvCapabilityDotProductInput4x8BitPackedKHR = 6018
    SpvCapabilityDotProduct = 6019
    SpvCapabilityDotProductKHR = 6019
    SpvCapabilityRayCullMaskKHR = 6020
    SpvCapabilityBitInstructions = 6025
    SpvCapabilityGroupNonUniformRotateKHR = 6026
    SpvCapabilityAtomicFloat32AddEXT = 6033
    SpvCapabilityAtomicFloat64AddEXT = 6034
    SpvCapabilityLongConstantCompositeINTEL = 6089
    SpvCapabilityOptNoneINTEL = 6094
    SpvCapabilityAtomicFloat16AddEXT = 6095
    SpvCapabilityDebugInfoModuleINTEL = 6114
    SpvCapabilitySplitBarrierINTEL = 6141
    SpvCapabilityGroupUniformArithmeticKHR = 6400
    SpvCapabilityMax = 2147483647
end

const SpvCapability = SpvCapability_

@cenum SpvRayFlagsShift_::UInt32 begin
    SpvRayFlagsOpaqueKHRShift = 0
    SpvRayFlagsNoOpaqueKHRShift = 1
    SpvRayFlagsTerminateOnFirstHitKHRShift = 2
    SpvRayFlagsSkipClosestHitShaderKHRShift = 3
    SpvRayFlagsCullBackFacingTrianglesKHRShift = 4
    SpvRayFlagsCullFrontFacingTrianglesKHRShift = 5
    SpvRayFlagsCullOpaqueKHRShift = 6
    SpvRayFlagsCullNoOpaqueKHRShift = 7
    SpvRayFlagsSkipTrianglesKHRShift = 8
    SpvRayFlagsSkipAABBsKHRShift = 9
    SpvRayFlagsMax = 2147483647
end

const SpvRayFlagsShift = SpvRayFlagsShift_

@cenum SpvRayFlagsMask_::UInt32 begin
    SpvRayFlagsMaskNone = 0
    SpvRayFlagsOpaqueKHRMask = 1
    SpvRayFlagsNoOpaqueKHRMask = 2
    SpvRayFlagsTerminateOnFirstHitKHRMask = 4
    SpvRayFlagsSkipClosestHitShaderKHRMask = 8
    SpvRayFlagsCullBackFacingTrianglesKHRMask = 16
    SpvRayFlagsCullFrontFacingTrianglesKHRMask = 32
    SpvRayFlagsCullOpaqueKHRMask = 64
    SpvRayFlagsCullNoOpaqueKHRMask = 128
    SpvRayFlagsSkipTrianglesKHRMask = 256
    SpvRayFlagsSkipAABBsKHRMask = 512
end

const SpvRayFlagsMask = SpvRayFlagsMask_

@cenum SpvRayQueryIntersection_::UInt32 begin
    SpvRayQueryIntersectionRayQueryCandidateIntersectionKHR = 0
    SpvRayQueryIntersectionRayQueryCommittedIntersectionKHR = 1
    SpvRayQueryIntersectionMax = 2147483647
end

const SpvRayQueryIntersection = SpvRayQueryIntersection_

@cenum SpvRayQueryCommittedIntersectionType_::UInt32 begin
    SpvRayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionNoneKHR = 0
    SpvRayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionTriangleKHR = 1
    SpvRayQueryCommittedIntersectionTypeRayQueryCommittedIntersectionGeneratedKHR = 2
    SpvRayQueryCommittedIntersectionTypeMax = 2147483647
end

const SpvRayQueryCommittedIntersectionType = SpvRayQueryCommittedIntersectionType_

@cenum SpvRayQueryCandidateIntersectionType_::UInt32 begin
    SpvRayQueryCandidateIntersectionTypeRayQueryCandidateIntersectionTriangleKHR = 0
    SpvRayQueryCandidateIntersectionTypeRayQueryCandidateIntersectionAABBKHR = 1
    SpvRayQueryCandidateIntersectionTypeMax = 2147483647
end

const SpvRayQueryCandidateIntersectionType = SpvRayQueryCandidateIntersectionType_

@cenum SpvFragmentShadingRateShift_::UInt32 begin
    SpvFragmentShadingRateVertical2PixelsShift = 0
    SpvFragmentShadingRateVertical4PixelsShift = 1
    SpvFragmentShadingRateHorizontal2PixelsShift = 2
    SpvFragmentShadingRateHorizontal4PixelsShift = 3
    SpvFragmentShadingRateMax = 2147483647
end

const SpvFragmentShadingRateShift = SpvFragmentShadingRateShift_

@cenum SpvFragmentShadingRateMask_::UInt32 begin
    SpvFragmentShadingRateMaskNone = 0
    SpvFragmentShadingRateVertical2PixelsMask = 1
    SpvFragmentShadingRateVertical4PixelsMask = 2
    SpvFragmentShadingRateHorizontal2PixelsMask = 4
    SpvFragmentShadingRateHorizontal4PixelsMask = 8
end

const SpvFragmentShadingRateMask = SpvFragmentShadingRateMask_

@cenum SpvFPDenormMode_::UInt32 begin
    SpvFPDenormModePreserve = 0
    SpvFPDenormModeFlushToZero = 1
    SpvFPDenormModeMax = 2147483647
end

const SpvFPDenormMode = SpvFPDenormMode_

@cenum SpvFPOperationMode_::UInt32 begin
    SpvFPOperationModeIEEE = 0
    SpvFPOperationModeALT = 1
    SpvFPOperationModeMax = 2147483647
end

const SpvFPOperationMode = SpvFPOperationMode_

@cenum SpvQuantizationModes_::UInt32 begin
    SpvQuantizationModesTRN = 0
    SpvQuantizationModesTRN_ZERO = 1
    SpvQuantizationModesRND = 2
    SpvQuantizationModesRND_ZERO = 3
    SpvQuantizationModesRND_INF = 4
    SpvQuantizationModesRND_MIN_INF = 5
    SpvQuantizationModesRND_CONV = 6
    SpvQuantizationModesRND_CONV_ODD = 7
    SpvQuantizationModesMax = 2147483647
end

const SpvQuantizationModes = SpvQuantizationModes_

@cenum SpvOverflowModes_::UInt32 begin
    SpvOverflowModesWRAP = 0
    SpvOverflowModesSAT = 1
    SpvOverflowModesSAT_ZERO = 2
    SpvOverflowModesSAT_SYM = 3
    SpvOverflowModesMax = 2147483647
end

const SpvOverflowModes = SpvOverflowModes_

@cenum SpvPackedVectorFormat_::UInt32 begin
    SpvPackedVectorFormatPackedVectorFormat4x8Bit = 0
    SpvPackedVectorFormatPackedVectorFormat4x8BitKHR = 0
    SpvPackedVectorFormatMax = 2147483647
end

const SpvPackedVectorFormat = SpvPackedVectorFormat_

@cenum SpvOp_::UInt32 begin
    SpvOpNop = 0
    SpvOpUndef = 1
    SpvOpSourceContinued = 2
    SpvOpSource = 3
    SpvOpSourceExtension = 4
    SpvOpName = 5
    SpvOpMemberName = 6
    SpvOpString = 7
    SpvOpLine = 8
    SpvOpExtension = 10
    SpvOpExtInstImport = 11
    SpvOpExtInst = 12
    SpvOpMemoryModel = 14
    SpvOpEntryPoint = 15
    SpvOpExecutionMode = 16
    SpvOpCapability = 17
    SpvOpTypeVoid = 19
    SpvOpTypeBool = 20
    SpvOpTypeInt = 21
    SpvOpTypeFloat = 22
    SpvOpTypeVector = 23
    SpvOpTypeMatrix = 24
    SpvOpTypeImage = 25
    SpvOpTypeSampler = 26
    SpvOpTypeSampledImage = 27
    SpvOpTypeArray = 28
    SpvOpTypeRuntimeArray = 29
    SpvOpTypeStruct = 30
    SpvOpTypeOpaque = 31
    SpvOpTypePointer = 32
    SpvOpTypeFunction = 33
    SpvOpTypeEvent = 34
    SpvOpTypeDeviceEvent = 35
    SpvOpTypeReserveId = 36
    SpvOpTypeQueue = 37
    SpvOpTypePipe = 38
    SpvOpTypeForwardPointer = 39
    SpvOpConstantTrue = 41
    SpvOpConstantFalse = 42
    SpvOpConstant = 43
    SpvOpConstantComposite = 44
    SpvOpConstantSampler = 45
    SpvOpConstantNull = 46
    SpvOpSpecConstantTrue = 48
    SpvOpSpecConstantFalse = 49
    SpvOpSpecConstant = 50
    SpvOpSpecConstantComposite = 51
    SpvOpSpecConstantOp = 52
    SpvOpFunction = 54
    SpvOpFunctionParameter = 55
    SpvOpFunctionEnd = 56
    SpvOpFunctionCall = 57
    SpvOpVariable = 59
    SpvOpImageTexelPointer = 60
    SpvOpLoad = 61
    SpvOpStore = 62
    SpvOpCopyMemory = 63
    SpvOpCopyMemorySized = 64
    SpvOpAccessChain = 65
    SpvOpInBoundsAccessChain = 66
    SpvOpPtrAccessChain = 67
    SpvOpArrayLength = 68
    SpvOpGenericPtrMemSemantics = 69
    SpvOpInBoundsPtrAccessChain = 70
    SpvOpDecorate = 71
    SpvOpMemberDecorate = 72
    SpvOpDecorationGroup = 73
    SpvOpGroupDecorate = 74
    SpvOpGroupMemberDecorate = 75
    SpvOpVectorExtractDynamic = 77
    SpvOpVectorInsertDynamic = 78
    SpvOpVectorShuffle = 79
    SpvOpCompositeConstruct = 80
    SpvOpCompositeExtract = 81
    SpvOpCompositeInsert = 82
    SpvOpCopyObject = 83
    SpvOpTranspose = 84
    SpvOpSampledImage = 86
    SpvOpImageSampleImplicitLod = 87
    SpvOpImageSampleExplicitLod = 88
    SpvOpImageSampleDrefImplicitLod = 89
    SpvOpImageSampleDrefExplicitLod = 90
    SpvOpImageSampleProjImplicitLod = 91
    SpvOpImageSampleProjExplicitLod = 92
    SpvOpImageSampleProjDrefImplicitLod = 93
    SpvOpImageSampleProjDrefExplicitLod = 94
    SpvOpImageFetch = 95
    SpvOpImageGather = 96
    SpvOpImageDrefGather = 97
    SpvOpImageRead = 98
    SpvOpImageWrite = 99
    SpvOpImage = 100
    SpvOpImageQueryFormat = 101
    SpvOpImageQueryOrder = 102
    SpvOpImageQuerySizeLod = 103
    SpvOpImageQuerySize = 104
    SpvOpImageQueryLod = 105
    SpvOpImageQueryLevels = 106
    SpvOpImageQuerySamples = 107
    SpvOpConvertFToU = 109
    SpvOpConvertFToS = 110
    SpvOpConvertSToF = 111
    SpvOpConvertUToF = 112
    SpvOpUConvert = 113
    SpvOpSConvert = 114
    SpvOpFConvert = 115
    SpvOpQuantizeToF16 = 116
    SpvOpConvertPtrToU = 117
    SpvOpSatConvertSToU = 118
    SpvOpSatConvertUToS = 119
    SpvOpConvertUToPtr = 120
    SpvOpPtrCastToGeneric = 121
    SpvOpGenericCastToPtr = 122
    SpvOpGenericCastToPtrExplicit = 123
    SpvOpBitcast = 124
    SpvOpSNegate = 126
    SpvOpFNegate = 127
    SpvOpIAdd = 128
    SpvOpFAdd = 129
    SpvOpISub = 130
    SpvOpFSub = 131
    SpvOpIMul = 132
    SpvOpFMul = 133
    SpvOpUDiv = 134
    SpvOpSDiv = 135
    SpvOpFDiv = 136
    SpvOpUMod = 137
    SpvOpSRem = 138
    SpvOpSMod = 139
    SpvOpFRem = 140
    SpvOpFMod = 141
    SpvOpVectorTimesScalar = 142
    SpvOpMatrixTimesScalar = 143
    SpvOpVectorTimesMatrix = 144
    SpvOpMatrixTimesVector = 145
    SpvOpMatrixTimesMatrix = 146
    SpvOpOuterProduct = 147
    SpvOpDot = 148
    SpvOpIAddCarry = 149
    SpvOpISubBorrow = 150
    SpvOpUMulExtended = 151
    SpvOpSMulExtended = 152
    SpvOpAny = 154
    SpvOpAll = 155
    SpvOpIsNan = 156
    SpvOpIsInf = 157
    SpvOpIsFinite = 158
    SpvOpIsNormal = 159
    SpvOpSignBitSet = 160
    SpvOpLessOrGreater = 161
    SpvOpOrdered = 162
    SpvOpUnordered = 163
    SpvOpLogicalEqual = 164
    SpvOpLogicalNotEqual = 165
    SpvOpLogicalOr = 166
    SpvOpLogicalAnd = 167
    SpvOpLogicalNot = 168
    SpvOpSelect = 169
    SpvOpIEqual = 170
    SpvOpINotEqual = 171
    SpvOpUGreaterThan = 172
    SpvOpSGreaterThan = 173
    SpvOpUGreaterThanEqual = 174
    SpvOpSGreaterThanEqual = 175
    SpvOpULessThan = 176
    SpvOpSLessThan = 177
    SpvOpULessThanEqual = 178
    SpvOpSLessThanEqual = 179
    SpvOpFOrdEqual = 180
    SpvOpFUnordEqual = 181
    SpvOpFOrdNotEqual = 182
    SpvOpFUnordNotEqual = 183
    SpvOpFOrdLessThan = 184
    SpvOpFUnordLessThan = 185
    SpvOpFOrdGreaterThan = 186
    SpvOpFUnordGreaterThan = 187
    SpvOpFOrdLessThanEqual = 188
    SpvOpFUnordLessThanEqual = 189
    SpvOpFOrdGreaterThanEqual = 190
    SpvOpFUnordGreaterThanEqual = 191
    SpvOpShiftRightLogical = 194
    SpvOpShiftRightArithmetic = 195
    SpvOpShiftLeftLogical = 196
    SpvOpBitwiseOr = 197
    SpvOpBitwiseXor = 198
    SpvOpBitwiseAnd = 199
    SpvOpNot = 200
    SpvOpBitFieldInsert = 201
    SpvOpBitFieldSExtract = 202
    SpvOpBitFieldUExtract = 203
    SpvOpBitReverse = 204
    SpvOpBitCount = 205
    SpvOpDPdx = 207
    SpvOpDPdy = 208
    SpvOpFwidth = 209
    SpvOpDPdxFine = 210
    SpvOpDPdyFine = 211
    SpvOpFwidthFine = 212
    SpvOpDPdxCoarse = 213
    SpvOpDPdyCoarse = 214
    SpvOpFwidthCoarse = 215
    SpvOpEmitVertex = 218
    SpvOpEndPrimitive = 219
    SpvOpEmitStreamVertex = 220
    SpvOpEndStreamPrimitive = 221
    SpvOpControlBarrier = 224
    SpvOpMemoryBarrier = 225
    SpvOpAtomicLoad = 227
    SpvOpAtomicStore = 228
    SpvOpAtomicExchange = 229
    SpvOpAtomicCompareExchange = 230
    SpvOpAtomicCompareExchangeWeak = 231
    SpvOpAtomicIIncrement = 232
    SpvOpAtomicIDecrement = 233
    SpvOpAtomicIAdd = 234
    SpvOpAtomicISub = 235
    SpvOpAtomicSMin = 236
    SpvOpAtomicUMin = 237
    SpvOpAtomicSMax = 238
    SpvOpAtomicUMax = 239
    SpvOpAtomicAnd = 240
    SpvOpAtomicOr = 241
    SpvOpAtomicXor = 242
    SpvOpPhi = 245
    SpvOpLoopMerge = 246
    SpvOpSelectionMerge = 247
    SpvOpLabel = 248
    SpvOpBranch = 249
    SpvOpBranchConditional = 250
    SpvOpSwitch = 251
    SpvOpKill = 252
    SpvOpReturn = 253
    SpvOpReturnValue = 254
    SpvOpUnreachable = 255
    SpvOpLifetimeStart = 256
    SpvOpLifetimeStop = 257
    SpvOpGroupAsyncCopy = 259
    SpvOpGroupWaitEvents = 260
    SpvOpGroupAll = 261
    SpvOpGroupAny = 262
    SpvOpGroupBroadcast = 263
    SpvOpGroupIAdd = 264
    SpvOpGroupFAdd = 265
    SpvOpGroupFMin = 266
    SpvOpGroupUMin = 267
    SpvOpGroupSMin = 268
    SpvOpGroupFMax = 269
    SpvOpGroupUMax = 270
    SpvOpGroupSMax = 271
    SpvOpReadPipe = 274
    SpvOpWritePipe = 275
    SpvOpReservedReadPipe = 276
    SpvOpReservedWritePipe = 277
    SpvOpReserveReadPipePackets = 278
    SpvOpReserveWritePipePackets = 279
    SpvOpCommitReadPipe = 280
    SpvOpCommitWritePipe = 281
    SpvOpIsValidReserveId = 282
    SpvOpGetNumPipePackets = 283
    SpvOpGetMaxPipePackets = 284
    SpvOpGroupReserveReadPipePackets = 285
    SpvOpGroupReserveWritePipePackets = 286
    SpvOpGroupCommitReadPipe = 287
    SpvOpGroupCommitWritePipe = 288
    SpvOpEnqueueMarker = 291
    SpvOpEnqueueKernel = 292
    SpvOpGetKernelNDrangeSubGroupCount = 293
    SpvOpGetKernelNDrangeMaxSubGroupSize = 294
    SpvOpGetKernelWorkGroupSize = 295
    SpvOpGetKernelPreferredWorkGroupSizeMultiple = 296
    SpvOpRetainEvent = 297
    SpvOpReleaseEvent = 298
    SpvOpCreateUserEvent = 299
    SpvOpIsValidEvent = 300
    SpvOpSetUserEventStatus = 301
    SpvOpCaptureEventProfilingInfo = 302
    SpvOpGetDefaultQueue = 303
    SpvOpBuildNDRange = 304
    SpvOpImageSparseSampleImplicitLod = 305
    SpvOpImageSparseSampleExplicitLod = 306
    SpvOpImageSparseSampleDrefImplicitLod = 307
    SpvOpImageSparseSampleDrefExplicitLod = 308
    SpvOpImageSparseSampleProjImplicitLod = 309
    SpvOpImageSparseSampleProjExplicitLod = 310
    SpvOpImageSparseSampleProjDrefImplicitLod = 311
    SpvOpImageSparseSampleProjDrefExplicitLod = 312
    SpvOpImageSparseFetch = 313
    SpvOpImageSparseGather = 314
    SpvOpImageSparseDrefGather = 315
    SpvOpImageSparseTexelsResident = 316
    SpvOpNoLine = 317
    SpvOpAtomicFlagTestAndSet = 318
    SpvOpAtomicFlagClear = 319
    SpvOpImageSparseRead = 320
    SpvOpSizeOf = 321
    SpvOpTypePipeStorage = 322
    SpvOpConstantPipeStorage = 323
    SpvOpCreatePipeFromPipeStorage = 324
    SpvOpGetKernelLocalSizeForSubgroupCount = 325
    SpvOpGetKernelMaxNumSubgroups = 326
    SpvOpTypeNamedBarrier = 327
    SpvOpNamedBarrierInitialize = 328
    SpvOpMemoryNamedBarrier = 329
    SpvOpModuleProcessed = 330
    SpvOpExecutionModeId = 331
    SpvOpDecorateId = 332
    SpvOpGroupNonUniformElect = 333
    SpvOpGroupNonUniformAll = 334
    SpvOpGroupNonUniformAny = 335
    SpvOpGroupNonUniformAllEqual = 336
    SpvOpGroupNonUniformBroadcast = 337
    SpvOpGroupNonUniformBroadcastFirst = 338
    SpvOpGroupNonUniformBallot = 339
    SpvOpGroupNonUniformInverseBallot = 340
    SpvOpGroupNonUniformBallotBitExtract = 341
    SpvOpGroupNonUniformBallotBitCount = 342
    SpvOpGroupNonUniformBallotFindLSB = 343
    SpvOpGroupNonUniformBallotFindMSB = 344
    SpvOpGroupNonUniformShuffle = 345
    SpvOpGroupNonUniformShuffleXor = 346
    SpvOpGroupNonUniformShuffleUp = 347
    SpvOpGroupNonUniformShuffleDown = 348
    SpvOpGroupNonUniformIAdd = 349
    SpvOpGroupNonUniformFAdd = 350
    SpvOpGroupNonUniformIMul = 351
    SpvOpGroupNonUniformFMul = 352
    SpvOpGroupNonUniformSMin = 353
    SpvOpGroupNonUniformUMin = 354
    SpvOpGroupNonUniformFMin = 355
    SpvOpGroupNonUniformSMax = 356
    SpvOpGroupNonUniformUMax = 357
    SpvOpGroupNonUniformFMax = 358
    SpvOpGroupNonUniformBitwiseAnd = 359
    SpvOpGroupNonUniformBitwiseOr = 360
    SpvOpGroupNonUniformBitwiseXor = 361
    SpvOpGroupNonUniformLogicalAnd = 362
    SpvOpGroupNonUniformLogicalOr = 363
    SpvOpGroupNonUniformLogicalXor = 364
    SpvOpGroupNonUniformQuadBroadcast = 365
    SpvOpGroupNonUniformQuadSwap = 366
    SpvOpCopyLogical = 400
    SpvOpPtrEqual = 401
    SpvOpPtrNotEqual = 402
    SpvOpPtrDiff = 403
    SpvOpTerminateInvocation = 4416
    SpvOpSubgroupBallotKHR = 4421
    SpvOpSubgroupFirstInvocationKHR = 4422
    SpvOpSubgroupAllKHR = 4428
    SpvOpSubgroupAnyKHR = 4429
    SpvOpSubgroupAllEqualKHR = 4430
    SpvOpGroupNonUniformRotateKHR = 4431
    SpvOpSubgroupReadInvocationKHR = 4432
    SpvOpTraceRayKHR = 4445
    SpvOpExecuteCallableKHR = 4446
    SpvOpConvertUToAccelerationStructureKHR = 4447
    SpvOpIgnoreIntersectionKHR = 4448
    SpvOpTerminateRayKHR = 4449
    SpvOpSDot = 4450
    SpvOpSDotKHR = 4450
    SpvOpUDot = 4451
    SpvOpUDotKHR = 4451
    SpvOpSUDot = 4452
    SpvOpSUDotKHR = 4452
    SpvOpSDotAccSat = 4453
    SpvOpSDotAccSatKHR = 4453
    SpvOpUDotAccSat = 4454
    SpvOpUDotAccSatKHR = 4454
    SpvOpSUDotAccSat = 4455
    SpvOpSUDotAccSatKHR = 4455
    SpvOpTypeRayQueryKHR = 4472
    SpvOpRayQueryInitializeKHR = 4473
    SpvOpRayQueryTerminateKHR = 4474
    SpvOpRayQueryGenerateIntersectionKHR = 4475
    SpvOpRayQueryConfirmIntersectionKHR = 4476
    SpvOpRayQueryProceedKHR = 4477
    SpvOpRayQueryGetIntersectionTypeKHR = 4479
    SpvOpGroupIAddNonUniformAMD = 5000
    SpvOpGroupFAddNonUniformAMD = 5001
    SpvOpGroupFMinNonUniformAMD = 5002
    SpvOpGroupUMinNonUniformAMD = 5003
    SpvOpGroupSMinNonUniformAMD = 5004
    SpvOpGroupFMaxNonUniformAMD = 5005
    SpvOpGroupUMaxNonUniformAMD = 5006
    SpvOpGroupSMaxNonUniformAMD = 5007
    SpvOpFragmentMaskFetchAMD = 5011
    SpvOpFragmentFetchAMD = 5012
    SpvOpReadClockKHR = 5056
    SpvOpImageSampleFootprintNV = 5283
    SpvOpGroupNonUniformPartitionNV = 5296
    SpvOpWritePackedPrimitiveIndices4x8NV = 5299
    SpvOpReportIntersectionKHR = 5334
    SpvOpReportIntersectionNV = 5334
    SpvOpIgnoreIntersectionNV = 5335
    SpvOpTerminateRayNV = 5336
    SpvOpTraceNV = 5337
    SpvOpTraceMotionNV = 5338
    SpvOpTraceRayMotionNV = 5339
    SpvOpTypeAccelerationStructureKHR = 5341
    SpvOpTypeAccelerationStructureNV = 5341
    SpvOpExecuteCallableNV = 5344
    SpvOpTypeCooperativeMatrixNV = 5358
    SpvOpCooperativeMatrixLoadNV = 5359
    SpvOpCooperativeMatrixStoreNV = 5360
    SpvOpCooperativeMatrixMulAddNV = 5361
    SpvOpCooperativeMatrixLengthNV = 5362
    SpvOpBeginInvocationInterlockEXT = 5364
    SpvOpEndInvocationInterlockEXT = 5365
    SpvOpDemoteToHelperInvocation = 5380
    SpvOpDemoteToHelperInvocationEXT = 5380
    SpvOpIsHelperInvocationEXT = 5381
    SpvOpConvertUToImageNV = 5391
    SpvOpConvertUToSamplerNV = 5392
    SpvOpConvertImageToUNV = 5393
    SpvOpConvertSamplerToUNV = 5394
    SpvOpConvertUToSampledImageNV = 5395
    SpvOpConvertSampledImageToUNV = 5396
    SpvOpSamplerImageAddressingModeNV = 5397
    SpvOpSubgroupShuffleINTEL = 5571
    SpvOpSubgroupShuffleDownINTEL = 5572
    SpvOpSubgroupShuffleUpINTEL = 5573
    SpvOpSubgroupShuffleXorINTEL = 5574
    SpvOpSubgroupBlockReadINTEL = 5575
    SpvOpSubgroupBlockWriteINTEL = 5576
    SpvOpSubgroupImageBlockReadINTEL = 5577
    SpvOpSubgroupImageBlockWriteINTEL = 5578
    SpvOpSubgroupImageMediaBlockReadINTEL = 5580
    SpvOpSubgroupImageMediaBlockWriteINTEL = 5581
    SpvOpUCountLeadingZerosINTEL = 5585
    SpvOpUCountTrailingZerosINTEL = 5586
    SpvOpAbsISubINTEL = 5587
    SpvOpAbsUSubINTEL = 5588
    SpvOpIAddSatINTEL = 5589
    SpvOpUAddSatINTEL = 5590
    SpvOpIAverageINTEL = 5591
    SpvOpUAverageINTEL = 5592
    SpvOpIAverageRoundedINTEL = 5593
    SpvOpUAverageRoundedINTEL = 5594
    SpvOpISubSatINTEL = 5595
    SpvOpUSubSatINTEL = 5596
    SpvOpIMul32x16INTEL = 5597
    SpvOpUMul32x16INTEL = 5598
    SpvOpConstantFunctionPointerINTEL = 5600
    SpvOpFunctionPointerCallINTEL = 5601
    SpvOpAsmTargetINTEL = 5609
    SpvOpAsmINTEL = 5610
    SpvOpAsmCallINTEL = 5611
    SpvOpAtomicFMinEXT = 5614
    SpvOpAtomicFMaxEXT = 5615
    SpvOpAssumeTrueKHR = 5630
    SpvOpExpectKHR = 5631
    SpvOpDecorateString = 5632
    SpvOpDecorateStringGOOGLE = 5632
    SpvOpMemberDecorateString = 5633
    SpvOpMemberDecorateStringGOOGLE = 5633
    SpvOpVmeImageINTEL = 5699
    SpvOpTypeVmeImageINTEL = 5700
    SpvOpTypeAvcImePayloadINTEL = 5701
    SpvOpTypeAvcRefPayloadINTEL = 5702
    SpvOpTypeAvcSicPayloadINTEL = 5703
    SpvOpTypeAvcMcePayloadINTEL = 5704
    SpvOpTypeAvcMceResultINTEL = 5705
    SpvOpTypeAvcImeResultINTEL = 5706
    SpvOpTypeAvcImeResultSingleReferenceStreamoutINTEL = 5707
    SpvOpTypeAvcImeResultDualReferenceStreamoutINTEL = 5708
    SpvOpTypeAvcImeSingleReferenceStreaminINTEL = 5709
    SpvOpTypeAvcImeDualReferenceStreaminINTEL = 5710
    SpvOpTypeAvcRefResultINTEL = 5711
    SpvOpTypeAvcSicResultINTEL = 5712
    SpvOpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL = 5713
    SpvOpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL = 5714
    SpvOpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL = 5715
    SpvOpSubgroupAvcMceSetInterShapePenaltyINTEL = 5716
    SpvOpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL = 5717
    SpvOpSubgroupAvcMceSetInterDirectionPenaltyINTEL = 5718
    SpvOpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL = 5719
    SpvOpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL = 5720
    SpvOpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL = 5721
    SpvOpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL = 5722
    SpvOpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL = 5723
    SpvOpSubgroupAvcMceSetMotionVectorCostFunctionINTEL = 5724
    SpvOpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL = 5725
    SpvOpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL = 5726
    SpvOpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL = 5727
    SpvOpSubgroupAvcMceSetAcOnlyHaarINTEL = 5728
    SpvOpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL = 5729
    SpvOpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL = 5730
    SpvOpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL = 5731
    SpvOpSubgroupAvcMceConvertToImePayloadINTEL = 5732
    SpvOpSubgroupAvcMceConvertToImeResultINTEL = 5733
    SpvOpSubgroupAvcMceConvertToRefPayloadINTEL = 5734
    SpvOpSubgroupAvcMceConvertToRefResultINTEL = 5735
    SpvOpSubgroupAvcMceConvertToSicPayloadINTEL = 5736
    SpvOpSubgroupAvcMceConvertToSicResultINTEL = 5737
    SpvOpSubgroupAvcMceGetMotionVectorsINTEL = 5738
    SpvOpSubgroupAvcMceGetInterDistortionsINTEL = 5739
    SpvOpSubgroupAvcMceGetBestInterDistortionsINTEL = 5740
    SpvOpSubgroupAvcMceGetInterMajorShapeINTEL = 5741
    SpvOpSubgroupAvcMceGetInterMinorShapeINTEL = 5742
    SpvOpSubgroupAvcMceGetInterDirectionsINTEL = 5743
    SpvOpSubgroupAvcMceGetInterMotionVectorCountINTEL = 5744
    SpvOpSubgroupAvcMceGetInterReferenceIdsINTEL = 5745
    SpvOpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL = 5746
    SpvOpSubgroupAvcImeInitializeINTEL = 5747
    SpvOpSubgroupAvcImeSetSingleReferenceINTEL = 5748
    SpvOpSubgroupAvcImeSetDualReferenceINTEL = 5749
    SpvOpSubgroupAvcImeRefWindowSizeINTEL = 5750
    SpvOpSubgroupAvcImeAdjustRefOffsetINTEL = 5751
    SpvOpSubgroupAvcImeConvertToMcePayloadINTEL = 5752
    SpvOpSubgroupAvcImeSetMaxMotionVectorCountINTEL = 5753
    SpvOpSubgroupAvcImeSetUnidirectionalMixDisableINTEL = 5754
    SpvOpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL = 5755
    SpvOpSubgroupAvcImeSetWeightedSadINTEL = 5756
    SpvOpSubgroupAvcImeEvaluateWithSingleReferenceINTEL = 5757
    SpvOpSubgroupAvcImeEvaluateWithDualReferenceINTEL = 5758
    SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL = 5759
    SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL = 5760
    SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL = 5761
    SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL = 5762
    SpvOpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL = 5763
    SpvOpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL = 5764
    SpvOpSubgroupAvcImeConvertToMceResultINTEL = 5765
    SpvOpSubgroupAvcImeGetSingleReferenceStreaminINTEL = 5766
    SpvOpSubgroupAvcImeGetDualReferenceStreaminINTEL = 5767
    SpvOpSubgroupAvcImeStripSingleReferenceStreamoutINTEL = 5768
    SpvOpSubgroupAvcImeStripDualReferenceStreamoutINTEL = 5769
    SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL = 5770
    SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL = 5771
    SpvOpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL = 5772
    SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL = 5773
    SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL = 5774
    SpvOpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL = 5775
    SpvOpSubgroupAvcImeGetBorderReachedINTEL = 5776
    SpvOpSubgroupAvcImeGetTruncatedSearchIndicationINTEL = 5777
    SpvOpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL = 5778
    SpvOpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL = 5779
    SpvOpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL = 5780
    SpvOpSubgroupAvcFmeInitializeINTEL = 5781
    SpvOpSubgroupAvcBmeInitializeINTEL = 5782
    SpvOpSubgroupAvcRefConvertToMcePayloadINTEL = 5783
    SpvOpSubgroupAvcRefSetBidirectionalMixDisableINTEL = 5784
    SpvOpSubgroupAvcRefSetBilinearFilterEnableINTEL = 5785
    SpvOpSubgroupAvcRefEvaluateWithSingleReferenceINTEL = 5786
    SpvOpSubgroupAvcRefEvaluateWithDualReferenceINTEL = 5787
    SpvOpSubgroupAvcRefEvaluateWithMultiReferenceINTEL = 5788
    SpvOpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL = 5789
    SpvOpSubgroupAvcRefConvertToMceResultINTEL = 5790
    SpvOpSubgroupAvcSicInitializeINTEL = 5791
    SpvOpSubgroupAvcSicConfigureSkcINTEL = 5792
    SpvOpSubgroupAvcSicConfigureIpeLumaINTEL = 5793
    SpvOpSubgroupAvcSicConfigureIpeLumaChromaINTEL = 5794
    SpvOpSubgroupAvcSicGetMotionVectorMaskINTEL = 5795
    SpvOpSubgroupAvcSicConvertToMcePayloadINTEL = 5796
    SpvOpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL = 5797
    SpvOpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL = 5798
    SpvOpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL = 5799
    SpvOpSubgroupAvcSicSetBilinearFilterEnableINTEL = 5800
    SpvOpSubgroupAvcSicSetSkcForwardTransformEnableINTEL = 5801
    SpvOpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL = 5802
    SpvOpSubgroupAvcSicEvaluateIpeINTEL = 5803
    SpvOpSubgroupAvcSicEvaluateWithSingleReferenceINTEL = 5804
    SpvOpSubgroupAvcSicEvaluateWithDualReferenceINTEL = 5805
    SpvOpSubgroupAvcSicEvaluateWithMultiReferenceINTEL = 5806
    SpvOpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL = 5807
    SpvOpSubgroupAvcSicConvertToMceResultINTEL = 5808
    SpvOpSubgroupAvcSicGetIpeLumaShapeINTEL = 5809
    SpvOpSubgroupAvcSicGetBestIpeLumaDistortionINTEL = 5810
    SpvOpSubgroupAvcSicGetBestIpeChromaDistortionINTEL = 5811
    SpvOpSubgroupAvcSicGetPackedIpeLumaModesINTEL = 5812
    SpvOpSubgroupAvcSicGetIpeChromaModeINTEL = 5813
    SpvOpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL = 5814
    SpvOpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL = 5815
    SpvOpSubgroupAvcSicGetInterRawSadsINTEL = 5816
    SpvOpVariableLengthArrayINTEL = 5818
    SpvOpSaveMemoryINTEL = 5819
    SpvOpRestoreMemoryINTEL = 5820
    SpvOpArbitraryFloatSinCosPiINTEL = 5840
    SpvOpArbitraryFloatCastINTEL = 5841
    SpvOpArbitraryFloatCastFromIntINTEL = 5842
    SpvOpArbitraryFloatCastToIntINTEL = 5843
    SpvOpArbitraryFloatAddINTEL = 5846
    SpvOpArbitraryFloatSubINTEL = 5847
    SpvOpArbitraryFloatMulINTEL = 5848
    SpvOpArbitraryFloatDivINTEL = 5849
    SpvOpArbitraryFloatGTINTEL = 5850
    SpvOpArbitraryFloatGEINTEL = 5851
    SpvOpArbitraryFloatLTINTEL = 5852
    SpvOpArbitraryFloatLEINTEL = 5853
    SpvOpArbitraryFloatEQINTEL = 5854
    SpvOpArbitraryFloatRecipINTEL = 5855
    SpvOpArbitraryFloatRSqrtINTEL = 5856
    SpvOpArbitraryFloatCbrtINTEL = 5857
    SpvOpArbitraryFloatHypotINTEL = 5858
    SpvOpArbitraryFloatSqrtINTEL = 5859
    SpvOpArbitraryFloatLogINTEL = 5860
    SpvOpArbitraryFloatLog2INTEL = 5861
    SpvOpArbitraryFloatLog10INTEL = 5862
    SpvOpArbitraryFloatLog1pINTEL = 5863
    SpvOpArbitraryFloatExpINTEL = 5864
    SpvOpArbitraryFloatExp2INTEL = 5865
    SpvOpArbitraryFloatExp10INTEL = 5866
    SpvOpArbitraryFloatExpm1INTEL = 5867
    SpvOpArbitraryFloatSinINTEL = 5868
    SpvOpArbitraryFloatCosINTEL = 5869
    SpvOpArbitraryFloatSinCosINTEL = 5870
    SpvOpArbitraryFloatSinPiINTEL = 5871
    SpvOpArbitraryFloatCosPiINTEL = 5872
    SpvOpArbitraryFloatASinINTEL = 5873
    SpvOpArbitraryFloatASinPiINTEL = 5874
    SpvOpArbitraryFloatACosINTEL = 5875
    SpvOpArbitraryFloatACosPiINTEL = 5876
    SpvOpArbitraryFloatATanINTEL = 5877
    SpvOpArbitraryFloatATanPiINTEL = 5878
    SpvOpArbitraryFloatATan2INTEL = 5879
    SpvOpArbitraryFloatPowINTEL = 5880
    SpvOpArbitraryFloatPowRINTEL = 5881
    SpvOpArbitraryFloatPowNINTEL = 5882
    SpvOpLoopControlINTEL = 5887
    SpvOpAliasDomainDeclINTEL = 5911
    SpvOpAliasScopeDeclINTEL = 5912
    SpvOpAliasScopeListDeclINTEL = 5913
    SpvOpFixedSqrtINTEL = 5923
    SpvOpFixedRecipINTEL = 5924
    SpvOpFixedRsqrtINTEL = 5925
    SpvOpFixedSinINTEL = 5926
    SpvOpFixedCosINTEL = 5927
    SpvOpFixedSinCosINTEL = 5928
    SpvOpFixedSinPiINTEL = 5929
    SpvOpFixedCosPiINTEL = 5930
    SpvOpFixedSinCosPiINTEL = 5931
    SpvOpFixedLogINTEL = 5932
    SpvOpFixedExpINTEL = 5933
    SpvOpPtrCastToCrossWorkgroupINTEL = 5934
    SpvOpCrossWorkgroupCastToPtrINTEL = 5938
    SpvOpReadPipeBlockingINTEL = 5946
    SpvOpWritePipeBlockingINTEL = 5947
    SpvOpFPGARegINTEL = 5949
    SpvOpRayQueryGetRayTMinKHR = 6016
    SpvOpRayQueryGetRayFlagsKHR = 6017
    SpvOpRayQueryGetIntersectionTKHR = 6018
    SpvOpRayQueryGetIntersectionInstanceCustomIndexKHR = 6019
    SpvOpRayQueryGetIntersectionInstanceIdKHR = 6020
    SpvOpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR = 6021
    SpvOpRayQueryGetIntersectionGeometryIndexKHR = 6022
    SpvOpRayQueryGetIntersectionPrimitiveIndexKHR = 6023
    SpvOpRayQueryGetIntersectionBarycentricsKHR = 6024
    SpvOpRayQueryGetIntersectionFrontFaceKHR = 6025
    SpvOpRayQueryGetIntersectionCandidateAABBOpaqueKHR = 6026
    SpvOpRayQueryGetIntersectionObjectRayDirectionKHR = 6027
    SpvOpRayQueryGetIntersectionObjectRayOriginKHR = 6028
    SpvOpRayQueryGetWorldRayDirectionKHR = 6029
    SpvOpRayQueryGetWorldRayOriginKHR = 6030
    SpvOpRayQueryGetIntersectionObjectToWorldKHR = 6031
    SpvOpRayQueryGetIntersectionWorldToObjectKHR = 6032
    SpvOpAtomicFAddEXT = 6035
    SpvOpTypeBufferSurfaceINTEL = 6086
    SpvOpTypeStructContinuedINTEL = 6090
    SpvOpConstantCompositeContinuedINTEL = 6091
    SpvOpSpecConstantCompositeContinuedINTEL = 6092
    SpvOpControlBarrierArriveINTEL = 6142
    SpvOpControlBarrierWaitINTEL = 6143
    SpvOpGroupIMulKHR = 6401
    SpvOpGroupFMulKHR = 6402
    SpvOpGroupBitwiseAndKHR = 6403
    SpvOpGroupBitwiseOrKHR = 6404
    SpvOpGroupBitwiseXorKHR = 6405
    SpvOpGroupLogicalAndKHR = 6406
    SpvOpGroupLogicalOrKHR = 6407
    SpvOpGroupLogicalXorKHR = 6408
    SpvOpMax = 2147483647
end

const SpvOp = SpvOp_

@cenum SpvReflectResult::UInt32 begin
    SPV_REFLECT_RESULT_SUCCESS = 0
    SPV_REFLECT_RESULT_NOT_READY = 1
    SPV_REFLECT_RESULT_ERROR_PARSE_FAILED = 2
    SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED = 3
    SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED = 4
    SPV_REFLECT_RESULT_ERROR_NULL_POINTER = 5
    SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR = 6
    SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH = 7
    SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND = 8
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE = 9
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER = 10
    SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF = 11
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE = 12
    SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW = 13
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS = 14
    SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION = 15
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION = 16
    SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA = 17
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE = 18
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT = 19
    SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE = 20
end

@cenum SpvReflectModuleFlagBits::UInt32 begin
    SPV_REFLECT_MODULE_FLAG_NONE = 0
    SPV_REFLECT_MODULE_FLAG_NO_COPY = 1
end

const SpvReflectModuleFlags = UInt32

@cenum SpvReflectTypeFlagBits::UInt32 begin
    SPV_REFLECT_TYPE_FLAG_UNDEFINED = 0
    SPV_REFLECT_TYPE_FLAG_VOID = 1
    SPV_REFLECT_TYPE_FLAG_BOOL = 2
    SPV_REFLECT_TYPE_FLAG_INT = 4
    SPV_REFLECT_TYPE_FLAG_FLOAT = 8
    SPV_REFLECT_TYPE_FLAG_VECTOR = 256
    SPV_REFLECT_TYPE_FLAG_MATRIX = 512
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE = 65536
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER = 131072
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE = 262144
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK = 524288
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_ACCELERATION_STRUCTURE = 1048576
    SPV_REFLECT_TYPE_FLAG_EXTERNAL_MASK = 16711680
    SPV_REFLECT_TYPE_FLAG_STRUCT = 268435456
    SPV_REFLECT_TYPE_FLAG_ARRAY = 536870912
end

const SpvReflectTypeFlags = UInt32

@cenum SpvReflectDecorationFlagBits::UInt32 begin
    SPV_REFLECT_DECORATION_NONE = 0
    SPV_REFLECT_DECORATION_BLOCK = 1
    SPV_REFLECT_DECORATION_BUFFER_BLOCK = 2
    SPV_REFLECT_DECORATION_ROW_MAJOR = 4
    SPV_REFLECT_DECORATION_COLUMN_MAJOR = 8
    SPV_REFLECT_DECORATION_BUILT_IN = 16
    SPV_REFLECT_DECORATION_NOPERSPECTIVE = 32
    SPV_REFLECT_DECORATION_FLAT = 64
    SPV_REFLECT_DECORATION_NON_WRITABLE = 128
    SPV_REFLECT_DECORATION_RELAXED_PRECISION = 256
    SPV_REFLECT_DECORATION_NON_READABLE = 512
end

const SpvReflectDecorationFlags = UInt32

@cenum SpvReflectResourceType::UInt32 begin
    SPV_REFLECT_RESOURCE_FLAG_UNDEFINED = 0
    SPV_REFLECT_RESOURCE_FLAG_SAMPLER = 1
    SPV_REFLECT_RESOURCE_FLAG_CBV = 2
    SPV_REFLECT_RESOURCE_FLAG_SRV = 4
    SPV_REFLECT_RESOURCE_FLAG_UAV = 8
end

@cenum SpvReflectFormat::UInt32 begin
    SPV_REFLECT_FORMAT_UNDEFINED = 0
    SPV_REFLECT_FORMAT_R32_UINT = 98
    SPV_REFLECT_FORMAT_R32_SINT = 99
    SPV_REFLECT_FORMAT_R32_SFLOAT = 100
    SPV_REFLECT_FORMAT_R32G32_UINT = 101
    SPV_REFLECT_FORMAT_R32G32_SINT = 102
    SPV_REFLECT_FORMAT_R32G32_SFLOAT = 103
    SPV_REFLECT_FORMAT_R32G32B32_UINT = 104
    SPV_REFLECT_FORMAT_R32G32B32_SINT = 105
    SPV_REFLECT_FORMAT_R32G32B32_SFLOAT = 106
    SPV_REFLECT_FORMAT_R32G32B32A32_UINT = 107
    SPV_REFLECT_FORMAT_R32G32B32A32_SINT = 108
    SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT = 109
    SPV_REFLECT_FORMAT_R64_UINT = 110
    SPV_REFLECT_FORMAT_R64_SINT = 111
    SPV_REFLECT_FORMAT_R64_SFLOAT = 112
    SPV_REFLECT_FORMAT_R64G64_UINT = 113
    SPV_REFLECT_FORMAT_R64G64_SINT = 114
    SPV_REFLECT_FORMAT_R64G64_SFLOAT = 115
    SPV_REFLECT_FORMAT_R64G64B64_UINT = 116
    SPV_REFLECT_FORMAT_R64G64B64_SINT = 117
    SPV_REFLECT_FORMAT_R64G64B64_SFLOAT = 118
    SPV_REFLECT_FORMAT_R64G64B64A64_UINT = 119
    SPV_REFLECT_FORMAT_R64G64B64A64_SINT = 120
    SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT = 121
end

@cenum SpvReflectVariableFlagBits::UInt32 begin
    SPV_REFLECT_VARIABLE_FLAGS_NONE = 0
    SPV_REFLECT_VARIABLE_FLAGS_UNUSED = 1
end

const SpvReflectVariableFlags = UInt32

@cenum SpvReflectDescriptorType::UInt32 begin
    SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER = 0
    SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1
    SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2
    SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3
    SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4
    SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5
    SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6
    SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7
    SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8
    SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9
    SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10
    SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000
end

@cenum SpvReflectShaderStageFlagBits::UInt32 begin
    SPV_REFLECT_SHADER_STAGE_VERTEX_BIT = 1
    SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 2
    SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 4
    SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT = 8
    SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT = 16
    SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT = 32
    SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV = 64
    SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV = 128
    SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR = 256
    SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR = 512
    SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 1024
    SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR = 2048
    SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR = 4096
    SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR = 8192
end

@cenum SpvReflectGenerator::UInt32 begin
    SPV_REFLECT_GENERATOR_KHRONOS_LLVM_SPIRV_TRANSLATOR = 6
    SPV_REFLECT_GENERATOR_KHRONOS_SPIRV_TOOLS_ASSEMBLER = 7
    SPV_REFLECT_GENERATOR_KHRONOS_GLSLANG_REFERENCE_FRONT_END = 8
    SPV_REFLECT_GENERATOR_GOOGLE_SHADERC_OVER_GLSLANG = 13
    SPV_REFLECT_GENERATOR_GOOGLE_SPIREGG = 14
    SPV_REFLECT_GENERATOR_GOOGLE_RSPIRV = 15
    SPV_REFLECT_GENERATOR_X_LEGEND_MESA_MESAIR_SPIRV_TRANSLATOR = 16
    SPV_REFLECT_GENERATOR_KHRONOS_SPIRV_TOOLS_LINKER = 17
    SPV_REFLECT_GENERATOR_WINE_VKD3D_SHADER_COMPILER = 18
    SPV_REFLECT_GENERATOR_CLAY_CLAY_SHADER_COMPILER = 19
end

@cenum var"##Ctag#298"::UInt32 begin
    SPV_REFLECT_MAX_ARRAY_DIMS = 32
    SPV_REFLECT_MAX_DESCRIPTOR_SETS = 64
end

@cenum var"##Ctag#299"::Int32 begin
    SPV_REFLECT_BINDING_NUMBER_DONT_CHANGE = -1
    SPV_REFLECT_SET_NUMBER_DONT_CHANGE = -1
end

struct Scalar
    width::UInt32
    signedness::UInt32
end

struct Vector
    component_count::UInt32
end

struct Matrix
    column_count::UInt32
    row_count::UInt32
    stride::UInt32
end

struct SpvReflectNumericTraits
    scalar::Scalar
    vector::Vector
    matrix::Matrix
end

struct SpvReflectImageTraits
    dim::SpvDim
    depth::UInt32
    arrayed::UInt32
    ms::UInt32
    sampled::UInt32
    image_format::SpvImageFormat
end

struct SpvReflectArrayTraits
    dims_count::UInt32
    dims::NTuple{32, UInt32}
    spec_constant_op_ids::NTuple{32, UInt32}
    stride::UInt32
end

struct SpvReflectBindingArrayTraits
    dims_count::UInt32
    dims::NTuple{32, UInt32}
end

struct Traits
    numeric::SpvReflectNumericTraits
    image::SpvReflectImageTraits
    array::SpvReflectArrayTraits
end

struct SpvReflectTypeDescription
    id::UInt32
    op::SpvOp
    type_name::Ptr{Cchar}
    struct_member_name::Ptr{Cchar}
    storage_class::SpvStorageClass
    type_flags::SpvReflectTypeFlags
    decoration_flags::SpvReflectDecorationFlags
    traits::Traits
    member_count::UInt32
    members::Ptr{SpvReflectTypeDescription}
end

struct var"##Ctag#301"
    location::UInt32
end
function Base.getproperty(x::Ptr{var"##Ctag#301"}, f::Symbol)
    f === :location && return Ptr{UInt32}(x + 0)
    return getfield(x, f)
end

function Base.getproperty(x::var"##Ctag#301", f::Symbol)
    r = Ref{var"##Ctag#301"}(x)
    ptr = Base.unsafe_convert(Ptr{var"##Ctag#301"}, r)
    fptr = getproperty(ptr, f)
    GC.@preserve r unsafe_load(fptr)
end

function Base.setproperty!(x::Ptr{var"##Ctag#301"}, f::Symbol, v)
    unsafe_store!(getproperty(x, f), v)
end


struct SpvReflectInterfaceVariable
    data::NTuple{368, UInt8}
end

function Base.getproperty(x::Ptr{SpvReflectInterfaceVariable}, f::Symbol)
    f === :spirv_id && return Ptr{UInt32}(x + 0)
    f === :name && return Ptr{Ptr{Cchar}}(x + 8)
    f === :location && return Ptr{UInt32}(x + 16)
    f === :storage_class && return Ptr{SpvStorageClass}(x + 20)
    f === :semantic && return Ptr{Ptr{Cchar}}(x + 24)
    f === :decoration_flags && return Ptr{SpvReflectDecorationFlags}(x + 32)
    f === :built_in && return Ptr{SpvBuiltIn}(x + 36)
    f === :numeric && return Ptr{SpvReflectNumericTraits}(x + 40)
    f === :array && return Ptr{SpvReflectArrayTraits}(x + 64)
    f === :member_count && return Ptr{UInt32}(x + 328)
    f === :members && return Ptr{Ptr{SpvReflectInterfaceVariable}}(x + 336)
    f === :format && return Ptr{SpvReflectFormat}(x + 344)
    f === :type_description && return Ptr{Ptr{SpvReflectTypeDescription}}(x + 352)
    f === :word_offset && return Ptr{var"##Ctag#301"}(x + 360)
    return getfield(x, f)
end

function Base.getproperty(x::SpvReflectInterfaceVariable, f::Symbol)
    r = Ref{SpvReflectInterfaceVariable}(x)
    ptr = Base.unsafe_convert(Ptr{SpvReflectInterfaceVariable}, r)
    fptr = getproperty(ptr, f)
    GC.@preserve r unsafe_load(fptr)
end

function Base.setproperty!(x::Ptr{SpvReflectInterfaceVariable}, f::Symbol, v)
    unsafe_store!(getproperty(x, f), v)
end

struct SpvReflectBlockVariable
    spirv_id::UInt32
    name::Ptr{Cchar}
    offset::UInt32
    absolute_offset::UInt32
    size::UInt32
    padded_size::UInt32
    decoration_flags::SpvReflectDecorationFlags
    numeric::SpvReflectNumericTraits
    array::SpvReflectArrayTraits
    flags::SpvReflectVariableFlags
    member_count::UInt32
    members::Ptr{SpvReflectBlockVariable}
    type_description::Ptr{SpvReflectTypeDescription}
end

struct var"##Ctag#300"
    binding::UInt32
    set::UInt32
end
function Base.getproperty(x::Ptr{var"##Ctag#300"}, f::Symbol)
    f === :binding && return Ptr{UInt32}(x + 0)
    f === :set && return Ptr{UInt32}(x + 4)
    return getfield(x, f)
end

function Base.getproperty(x::var"##Ctag#300", f::Symbol)
    r = Ref{var"##Ctag#300"}(x)
    ptr = Base.unsafe_convert(Ptr{var"##Ctag#300"}, r)
    fptr = getproperty(ptr, f)
    GC.@preserve r unsafe_load(fptr)
end

function Base.setproperty!(x::Ptr{var"##Ctag#300"}, f::Symbol, v)
    unsafe_store!(getproperty(x, f), v)
end


struct SpvReflectDescriptorBinding
    data::NTuple{592, UInt8}
end

function Base.getproperty(x::Ptr{SpvReflectDescriptorBinding}, f::Symbol)
    f === :spirv_id && return Ptr{UInt32}(x + 0)
    f === :name && return Ptr{Ptr{Cchar}}(x + 8)
    f === :binding && return Ptr{UInt32}(x + 16)
    f === :input_attachment_index && return Ptr{UInt32}(x + 20)
    f === :set && return Ptr{UInt32}(x + 24)
    f === :descriptor_type && return Ptr{SpvReflectDescriptorType}(x + 28)
    f === :resource_type && return Ptr{SpvReflectResourceType}(x + 32)
    f === :image && return Ptr{SpvReflectImageTraits}(x + 36)
    f === :block && return Ptr{SpvReflectBlockVariable}(x + 64)
    f === :array && return Ptr{SpvReflectBindingArrayTraits}(x + 416)
    f === :count && return Ptr{UInt32}(x + 548)
    f === :accessed && return Ptr{UInt32}(x + 552)
    f === :uav_counter_id && return Ptr{UInt32}(x + 556)
    f === :uav_counter_binding && return Ptr{Ptr{SpvReflectDescriptorBinding}}(x + 560)
    f === :type_description && return Ptr{Ptr{SpvReflectTypeDescription}}(x + 568)
    f === :word_offset && return Ptr{var"##Ctag#300"}(x + 576)
    f === :decoration_flags && return Ptr{SpvReflectDecorationFlags}(x + 584)
    return getfield(x, f)
end

function Base.getproperty(x::SpvReflectDescriptorBinding, f::Symbol)
    r = Ref{SpvReflectDescriptorBinding}(x)
    ptr = Base.unsafe_convert(Ptr{SpvReflectDescriptorBinding}, r)
    fptr = getproperty(ptr, f)
    GC.@preserve r unsafe_load(fptr)
end

function Base.setproperty!(x::Ptr{SpvReflectDescriptorBinding}, f::Symbol, v)
    unsafe_store!(getproperty(x, f), v)
end

struct SpvReflectDescriptorSet
    set::UInt32
    binding_count::UInt32
    bindings::Ptr{Ptr{SpvReflectDescriptorBinding}}
end

struct LocalSize
    x::UInt32
    y::UInt32
    z::UInt32
end

struct SpvReflectEntryPoint
    name::Ptr{Cchar}
    id::UInt32
    spirv_execution_model::SpvExecutionModel
    shader_stage::SpvReflectShaderStageFlagBits
    input_variable_count::UInt32
    input_variables::Ptr{Ptr{SpvReflectInterfaceVariable}}
    output_variable_count::UInt32
    output_variables::Ptr{Ptr{SpvReflectInterfaceVariable}}
    interface_variable_count::UInt32
    interface_variables::Ptr{SpvReflectInterfaceVariable}
    descriptor_set_count::UInt32
    descriptor_sets::Ptr{SpvReflectDescriptorSet}
    used_uniform_count::UInt32
    used_uniforms::Ptr{UInt32}
    used_push_constant_count::UInt32
    used_push_constants::Ptr{UInt32}
    execution_mode_count::UInt32
    execution_modes::Ptr{SpvExecutionMode}
    local_size::LocalSize
    invocations::UInt32
    output_vertices::UInt32
end

struct SpvReflectCapability
    value::SpvCapability
    word_offset::UInt32
end

struct Internal
    module_flags::SpvReflectModuleFlags
    spirv_size::Csize_t
    spirv_code::Ptr{UInt32}
    spirv_word_count::UInt32
    type_description_count::Csize_t
    type_descriptions::Ptr{SpvReflectTypeDescription}
end

struct SpvReflectShaderModule
    generator::SpvReflectGenerator
    entry_point_name::Ptr{Cchar}
    entry_point_id::UInt32
    entry_point_count::UInt32
    entry_points::Ptr{SpvReflectEntryPoint}
    source_language::SpvSourceLanguage
    source_language_version::UInt32
    source_file::Ptr{Cchar}
    source_source::Ptr{Cchar}
    capability_count::UInt32
    capabilities::Ptr{SpvReflectCapability}
    spirv_execution_model::SpvExecutionModel
    shader_stage::SpvReflectShaderStageFlagBits
    descriptor_binding_count::UInt32
    descriptor_bindings::Ptr{SpvReflectDescriptorBinding}
    descriptor_set_count::UInt32
    descriptor_sets::NTuple{64, SpvReflectDescriptorSet}
    input_variable_count::UInt32
    input_variables::Ptr{Ptr{SpvReflectInterfaceVariable}}
    output_variable_count::UInt32
    output_variables::Ptr{Ptr{SpvReflectInterfaceVariable}}
    interface_variable_count::UInt32
    interface_variables::Ptr{SpvReflectInterfaceVariable}
    push_constant_block_count::UInt32
    push_constant_blocks::Ptr{SpvReflectBlockVariable}
    _internal::Ptr{Internal}
end

function spvReflectCreateShaderModule(size, p_code, p_module)
    ccall((:spvReflectCreateShaderModule, spvreflect), SpvReflectResult, (Csize_t, Ptr{Cvoid}, Ptr{SpvReflectShaderModule}), size, p_code, p_module)
end

function spvReflectCreateShaderModule2(flags, size, p_code, p_module)
    ccall((:spvReflectCreateShaderModule2, spvreflect), SpvReflectResult, (SpvReflectModuleFlags, Csize_t, Ptr{Cvoid}, Ptr{SpvReflectShaderModule}), flags, size, p_code, p_module)
end

function spvReflectGetShaderModule(size, p_code, p_module)
    ccall((:spvReflectGetShaderModule, spvreflect), SpvReflectResult, (Csize_t, Ptr{Cvoid}, Ptr{SpvReflectShaderModule}), size, p_code, p_module)
end

function spvReflectDestroyShaderModule(p_module)
    ccall((:spvReflectDestroyShaderModule, spvreflect), Cvoid, (Ptr{SpvReflectShaderModule},), p_module)
end

function spvReflectGetCodeSize(p_module)
    ccall((:spvReflectGetCodeSize, spvreflect), UInt32, (Ptr{SpvReflectShaderModule},), p_module)
end

function spvReflectGetCode(p_module)
    ccall((:spvReflectGetCode, spvreflect), Ptr{UInt32}, (Ptr{SpvReflectShaderModule},), p_module)
end

function spvReflectGetEntryPoint(p_module, entry_point)
    ccall((:spvReflectGetEntryPoint, spvreflect), Ptr{SpvReflectEntryPoint}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}), p_module, entry_point)
end

function spvReflectEnumerateDescriptorBindings(p_module, p_count, pp_bindings)
    ccall((:spvReflectEnumerateDescriptorBindings, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectDescriptorBinding}}), p_module, p_count, pp_bindings)
end

function spvReflectEnumerateEntryPointDescriptorBindings(p_module, entry_point, p_count, pp_bindings)
    ccall((:spvReflectEnumerateEntryPointDescriptorBindings, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectDescriptorBinding}}), p_module, entry_point, p_count, pp_bindings)
end

function spvReflectEnumerateDescriptorSets(p_module, p_count, pp_sets)
    ccall((:spvReflectEnumerateDescriptorSets, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectDescriptorSet}}), p_module, p_count, pp_sets)
end

function spvReflectEnumerateEntryPointDescriptorSets(p_module, entry_point, p_count, pp_sets)
    ccall((:spvReflectEnumerateEntryPointDescriptorSets, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectDescriptorSet}}), p_module, entry_point, p_count, pp_sets)
end

function spvReflectEnumerateInterfaceVariables(p_module, p_count, pp_variables)
    ccall((:spvReflectEnumerateInterfaceVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, p_count, pp_variables)
end

function spvReflectEnumerateEntryPointInterfaceVariables(p_module, entry_point, p_count, pp_variables)
    ccall((:spvReflectEnumerateEntryPointInterfaceVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, entry_point, p_count, pp_variables)
end

function spvReflectEnumerateInputVariables(p_module, p_count, pp_variables)
    ccall((:spvReflectEnumerateInputVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, p_count, pp_variables)
end

function spvReflectEnumerateEntryPointInputVariables(p_module, entry_point, p_count, pp_variables)
    ccall((:spvReflectEnumerateEntryPointInputVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, entry_point, p_count, pp_variables)
end

function spvReflectEnumerateOutputVariables(p_module, p_count, pp_variables)
    ccall((:spvReflectEnumerateOutputVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, p_count, pp_variables)
end

function spvReflectEnumerateEntryPointOutputVariables(p_module, entry_point, p_count, pp_variables)
    ccall((:spvReflectEnumerateEntryPointOutputVariables, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectInterfaceVariable}}), p_module, entry_point, p_count, pp_variables)
end

function spvReflectEnumeratePushConstantBlocks(p_module, p_count, pp_blocks)
    ccall((:spvReflectEnumeratePushConstantBlocks, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectBlockVariable}}), p_module, p_count, pp_blocks)
end

function spvReflectEnumeratePushConstants(p_module, p_count, pp_blocks)
    ccall((:spvReflectEnumeratePushConstants, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{UInt32}, Ptr{Ptr{SpvReflectBlockVariable}}), p_module, p_count, pp_blocks)
end

function spvReflectEnumerateEntryPointPushConstantBlocks(p_module, entry_point, p_count, pp_blocks)
    ccall((:spvReflectEnumerateEntryPointPushConstantBlocks, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{UInt32}, Ptr{Ptr{SpvReflectBlockVariable}}), p_module, entry_point, p_count, pp_blocks)
end

function spvReflectGetDescriptorBinding(p_module, binding_number, set_number, p_result)
    ccall((:spvReflectGetDescriptorBinding, spvreflect), Ptr{SpvReflectDescriptorBinding}, (Ptr{SpvReflectShaderModule}, UInt32, UInt32, Ptr{SpvReflectResult}), p_module, binding_number, set_number, p_result)
end

function spvReflectGetEntryPointDescriptorBinding(p_module, entry_point, binding_number, set_number, p_result)
    ccall((:spvReflectGetEntryPointDescriptorBinding, spvreflect), Ptr{SpvReflectDescriptorBinding}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, UInt32, UInt32, Ptr{SpvReflectResult}), p_module, entry_point, binding_number, set_number, p_result)
end

function spvReflectGetDescriptorSet(p_module, set_number, p_result)
    ccall((:spvReflectGetDescriptorSet, spvreflect), Ptr{SpvReflectDescriptorSet}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, set_number, p_result)
end

function spvReflectGetEntryPointDescriptorSet(p_module, entry_point, set_number, p_result)
    ccall((:spvReflectGetEntryPointDescriptorSet, spvreflect), Ptr{SpvReflectDescriptorSet}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, UInt32, Ptr{SpvReflectResult}), p_module, entry_point, set_number, p_result)
end

function spvReflectGetInputVariableByLocation(p_module, location, p_result)
    ccall((:spvReflectGetInputVariableByLocation, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, location, p_result)
end

function spvReflectGetInputVariable(p_module, location, p_result)
    ccall((:spvReflectGetInputVariable, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, location, p_result)
end

function spvReflectGetEntryPointInputVariableByLocation(p_module, entry_point, location, p_result)
    ccall((:spvReflectGetEntryPointInputVariableByLocation, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, UInt32, Ptr{SpvReflectResult}), p_module, entry_point, location, p_result)
end

function spvReflectGetInputVariableBySemantic(p_module, semantic, p_result)
    ccall((:spvReflectGetInputVariableBySemantic, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{SpvReflectResult}), p_module, semantic, p_result)
end

function spvReflectGetEntryPointInputVariableBySemantic(p_module, entry_point, semantic, p_result)
    ccall((:spvReflectGetEntryPointInputVariableBySemantic, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{Cchar}, Ptr{SpvReflectResult}), p_module, entry_point, semantic, p_result)
end

function spvReflectGetOutputVariableByLocation(p_module, location, p_result)
    ccall((:spvReflectGetOutputVariableByLocation, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, location, p_result)
end

function spvReflectGetOutputVariable(p_module, location, p_result)
    ccall((:spvReflectGetOutputVariable, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, location, p_result)
end

function spvReflectGetEntryPointOutputVariableByLocation(p_module, entry_point, location, p_result)
    ccall((:spvReflectGetEntryPointOutputVariableByLocation, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, UInt32, Ptr{SpvReflectResult}), p_module, entry_point, location, p_result)
end

function spvReflectGetOutputVariableBySemantic(p_module, semantic, p_result)
    ccall((:spvReflectGetOutputVariableBySemantic, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{SpvReflectResult}), p_module, semantic, p_result)
end

function spvReflectGetEntryPointOutputVariableBySemantic(p_module, entry_point, semantic, p_result)
    ccall((:spvReflectGetEntryPointOutputVariableBySemantic, spvreflect), Ptr{SpvReflectInterfaceVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{Cchar}, Ptr{SpvReflectResult}), p_module, entry_point, semantic, p_result)
end

function spvReflectGetPushConstantBlock(p_module, index, p_result)
    ccall((:spvReflectGetPushConstantBlock, spvreflect), Ptr{SpvReflectBlockVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, index, p_result)
end

function spvReflectGetPushConstant(p_module, index, p_result)
    ccall((:spvReflectGetPushConstant, spvreflect), Ptr{SpvReflectBlockVariable}, (Ptr{SpvReflectShaderModule}, UInt32, Ptr{SpvReflectResult}), p_module, index, p_result)
end

function spvReflectGetEntryPointPushConstantBlock(p_module, entry_point, p_result)
    ccall((:spvReflectGetEntryPointPushConstantBlock, spvreflect), Ptr{SpvReflectBlockVariable}, (Ptr{SpvReflectShaderModule}, Ptr{Cchar}, Ptr{SpvReflectResult}), p_module, entry_point, p_result)
end

function spvReflectChangeDescriptorBindingNumbers(p_module, p_binding, new_binding_number, new_set_number)
    ccall((:spvReflectChangeDescriptorBindingNumbers, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{SpvReflectDescriptorBinding}, UInt32, UInt32), p_module, p_binding, new_binding_number, new_set_number)
end

function spvReflectChangeDescriptorBindingNumber(p_module, p_descriptor_binding, new_binding_number, optional_new_set_number)
    ccall((:spvReflectChangeDescriptorBindingNumber, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{SpvReflectDescriptorBinding}, UInt32, UInt32), p_module, p_descriptor_binding, new_binding_number, optional_new_set_number)
end

function spvReflectChangeDescriptorSetNumber(p_module, p_set, new_set_number)
    ccall((:spvReflectChangeDescriptorSetNumber, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{SpvReflectDescriptorSet}, UInt32), p_module, p_set, new_set_number)
end

function spvReflectChangeInputVariableLocation(p_module, p_input_variable, new_location)
    ccall((:spvReflectChangeInputVariableLocation, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{SpvReflectInterfaceVariable}, UInt32), p_module, p_input_variable, new_location)
end

function spvReflectChangeOutputVariableLocation(p_module, p_output_variable, new_location)
    ccall((:spvReflectChangeOutputVariableLocation, spvreflect), SpvReflectResult, (Ptr{SpvReflectShaderModule}, Ptr{SpvReflectInterfaceVariable}, UInt32), p_module, p_output_variable, new_location)
end

function spvReflectSourceLanguage(source_lang)
    ccall((:spvReflectSourceLanguage, spvreflect), Ptr{Cchar}, (SpvSourceLanguage,), source_lang)
end

function spvReflectBlockVariableTypeName(p_var)
    ccall((:spvReflectBlockVariableTypeName, spvreflect), Ptr{Cchar}, (Ptr{SpvReflectBlockVariable},), p_var)
end

const SPV_VERSION = 0x00010600

const SPV_REVISION = 1

# exports
const PREFIXES = ["spv"]
for name in names(@__MODULE__; all=true), prefix in PREFIXES
    if startswith(string(name), prefix)
        @eval export $name
    end
end

end # module
