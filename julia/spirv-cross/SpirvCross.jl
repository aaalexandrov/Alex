module SpirvCross

using CEnum

@static if Sys.islinux()
    const spirv_cross_c_shared="libspirv-cross-c-shared"
end
spirv_cross_c_shared_handle = Base.Libc.Libdl.dlopen("libspirv-cross-c-shared")

const spvc_bool = Cuchar

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
    SpvExecutionModelTaskEXT = 5364
    SpvExecutionModelMeshEXT = 5365
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
    SpvExecutionModeEarlyAndLateFragmentTestsAMD = 5017
    SpvExecutionModeStencilRefReplacingEXT = 5027
    SpvExecutionModeStencilRefUnchangedFrontAMD = 5079
    SpvExecutionModeStencilRefGreaterFrontAMD = 5080
    SpvExecutionModeStencilRefLessFrontAMD = 5081
    SpvExecutionModeStencilRefUnchangedBackAMD = 5082
    SpvExecutionModeStencilRefGreaterBackAMD = 5083
    SpvExecutionModeStencilRefLessBackAMD = 5084
    SpvExecutionModeOutputLinesEXT = 5269
    SpvExecutionModeOutputLinesNV = 5269
    SpvExecutionModeOutputPrimitivesEXT = 5270
    SpvExecutionModeOutputPrimitivesNV = 5270
    SpvExecutionModeDerivativeGroupQuadsNV = 5289
    SpvExecutionModeDerivativeGroupLinearNV = 5290
    SpvExecutionModeOutputTrianglesEXT = 5298
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
    SpvStorageClassTaskPayloadWorkgroupEXT = 5402
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
    SpvDecorationPerPrimitiveEXT = 5271
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
    SpvBuiltInPrimitivePointIndicesEXT = 5294
    SpvBuiltInPrimitiveLineIndicesEXT = 5295
    SpvBuiltInPrimitiveTriangleIndicesEXT = 5296
    SpvBuiltInCullPrimitiveEXT = 5299
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
    SpvCapabilityMeshShadingEXT = 5283
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
    SpvOpEmitMeshTasksEXT = 5294
    SpvOpSetMeshOutputsEXT = 5295
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

function spvc_get_version(major, minor, patch)
    ccall((:spvc_get_version, spirv_cross_c_shared), Cvoid, (Ptr{Cuint}, Ptr{Cuint}, Ptr{Cuint}), major, minor, patch)
end

function spvc_get_commit_revision_and_timestamp()
    ccall((:spvc_get_commit_revision_and_timestamp, spirv_cross_c_shared), Ptr{Cchar}, ())
end

mutable struct spvc_context_s end

const spvc_context = Ptr{spvc_context_s}

mutable struct spvc_parsed_ir_s end

const spvc_parsed_ir = Ptr{spvc_parsed_ir_s}

mutable struct spvc_compiler_s end

const spvc_compiler = Ptr{spvc_compiler_s}

mutable struct spvc_compiler_options_s end

const spvc_compiler_options = Ptr{spvc_compiler_options_s}

mutable struct spvc_resources_s end

const spvc_resources = Ptr{spvc_resources_s}

mutable struct spvc_type_s end

const spvc_type = Ptr{spvc_type_s}

mutable struct spvc_constant_s end

const spvc_constant = Ptr{spvc_constant_s}

mutable struct spvc_set_s end

const spvc_set = Ptr{spvc_set_s}

const spvc_type_id = SpvId

const spvc_variable_id = SpvId

const spvc_constant_id = SpvId

struct spvc_reflected_resource
    id::spvc_variable_id
    base_type_id::spvc_type_id
    type_id::spvc_type_id
    name::Ptr{Cchar}
end

struct spvc_reflected_builtin_resource
    builtin::SpvBuiltIn
    value_type_id::spvc_type_id
    resource::spvc_reflected_resource
end

struct spvc_entry_point
    execution_model::SpvExecutionModel
    name::Ptr{Cchar}
end

struct spvc_combined_image_sampler
    combined_id::spvc_variable_id
    image_id::spvc_variable_id
    sampler_id::spvc_variable_id
end

struct spvc_specialization_constant
    id::spvc_constant_id
    constant_id::Cuint
end

struct spvc_buffer_range
    index::Cuint
    offset::Csize_t
    range::Csize_t
end

struct spvc_hlsl_root_constants
    start::Cuint
    _end::Cuint
    binding::Cuint
    space::Cuint
end

struct spvc_hlsl_vertex_attribute_remap
    location::Cuint
    semantic::Ptr{Cchar}
end

@cenum spvc_result::Int32 begin
    SPVC_SUCCESS = 0
    SPVC_ERROR_INVALID_SPIRV = -1
    SPVC_ERROR_UNSUPPORTED_SPIRV = -2
    SPVC_ERROR_OUT_OF_MEMORY = -3
    SPVC_ERROR_INVALID_ARGUMENT = -4
    SPVC_ERROR_INT_MAX = 2147483647
end

@cenum spvc_capture_mode::UInt32 begin
    SPVC_CAPTURE_MODE_COPY = 0
    SPVC_CAPTURE_MODE_TAKE_OWNERSHIP = 1
    SPVC_CAPTURE_MODE_INT_MAX = 2147483647
end

@cenum spvc_backend::UInt32 begin
    SPVC_BACKEND_NONE = 0
    SPVC_BACKEND_GLSL = 1
    SPVC_BACKEND_HLSL = 2
    SPVC_BACKEND_MSL = 3
    SPVC_BACKEND_CPP = 4
    SPVC_BACKEND_JSON = 5
    SPVC_BACKEND_INT_MAX = 2147483647
end

@cenum spvc_resource_type::UInt32 begin
    SPVC_RESOURCE_TYPE_UNKNOWN = 0
    SPVC_RESOURCE_TYPE_UNIFORM_BUFFER = 1
    SPVC_RESOURCE_TYPE_STORAGE_BUFFER = 2
    SPVC_RESOURCE_TYPE_STAGE_INPUT = 3
    SPVC_RESOURCE_TYPE_STAGE_OUTPUT = 4
    SPVC_RESOURCE_TYPE_SUBPASS_INPUT = 5
    SPVC_RESOURCE_TYPE_STORAGE_IMAGE = 6
    SPVC_RESOURCE_TYPE_SAMPLED_IMAGE = 7
    SPVC_RESOURCE_TYPE_ATOMIC_COUNTER = 8
    SPVC_RESOURCE_TYPE_PUSH_CONSTANT = 9
    SPVC_RESOURCE_TYPE_SEPARATE_IMAGE = 10
    SPVC_RESOURCE_TYPE_SEPARATE_SAMPLERS = 11
    SPVC_RESOURCE_TYPE_ACCELERATION_STRUCTURE = 12
    SPVC_RESOURCE_TYPE_RAY_QUERY = 13
    SPVC_RESOURCE_TYPE_SHADER_RECORD_BUFFER = 14
    SPVC_RESOURCE_TYPE_INT_MAX = 2147483647
end

@cenum spvc_builtin_resource_type::UInt32 begin
    SPVC_BUILTIN_RESOURCE_TYPE_UNKNOWN = 0
    SPVC_BUILTIN_RESOURCE_TYPE_STAGE_INPUT = 1
    SPVC_BUILTIN_RESOURCE_TYPE_STAGE_OUTPUT = 2
    SPVC_BUILTIN_RESOURCE_TYPE_INT_MAX = 2147483647
end

@cenum spvc_basetype::UInt32 begin
    SPVC_BASETYPE_UNKNOWN = 0
    SPVC_BASETYPE_VOID = 1
    SPVC_BASETYPE_BOOLEAN = 2
    SPVC_BASETYPE_INT8 = 3
    SPVC_BASETYPE_UINT8 = 4
    SPVC_BASETYPE_INT16 = 5
    SPVC_BASETYPE_UINT16 = 6
    SPVC_BASETYPE_INT32 = 7
    SPVC_BASETYPE_UINT32 = 8
    SPVC_BASETYPE_INT64 = 9
    SPVC_BASETYPE_UINT64 = 10
    SPVC_BASETYPE_ATOMIC_COUNTER = 11
    SPVC_BASETYPE_FP16 = 12
    SPVC_BASETYPE_FP32 = 13
    SPVC_BASETYPE_FP64 = 14
    SPVC_BASETYPE_STRUCT = 15
    SPVC_BASETYPE_IMAGE = 16
    SPVC_BASETYPE_SAMPLED_IMAGE = 17
    SPVC_BASETYPE_SAMPLER = 18
    SPVC_BASETYPE_ACCELERATION_STRUCTURE = 19
    SPVC_BASETYPE_INT_MAX = 2147483647
end

@cenum spvc_msl_platform::UInt32 begin
    SPVC_MSL_PLATFORM_IOS = 0
    SPVC_MSL_PLATFORM_MACOS = 1
    SPVC_MSL_PLATFORM_MAX_INT = 2147483647
end

@cenum spvc_msl_index_type::UInt32 begin
    SPVC_MSL_INDEX_TYPE_NONE = 0
    SPVC_MSL_INDEX_TYPE_UINT16 = 1
    SPVC_MSL_INDEX_TYPE_UINT32 = 2
    SPVC_MSL_INDEX_TYPE_MAX_INT = 2147483647
end

@cenum spvc_msl_shader_variable_format::UInt32 begin
    SPVC_MSL_SHADER_VARIABLE_FORMAT_OTHER = 0
    SPVC_MSL_SHADER_VARIABLE_FORMAT_UINT8 = 1
    SPVC_MSL_SHADER_VARIABLE_FORMAT_UINT16 = 2
    SPVC_MSL_SHADER_VARIABLE_FORMAT_ANY16 = 3
    SPVC_MSL_SHADER_VARIABLE_FORMAT_ANY32 = 4
    SPVC_MSL_VERTEX_FORMAT_OTHER = 0
    SPVC_MSL_VERTEX_FORMAT_UINT8 = 1
    SPVC_MSL_VERTEX_FORMAT_UINT16 = 2
    SPVC_MSL_SHADER_INPUT_FORMAT_OTHER = 0
    SPVC_MSL_SHADER_INPUT_FORMAT_UINT8 = 1
    SPVC_MSL_SHADER_INPUT_FORMAT_UINT16 = 2
    SPVC_MSL_SHADER_INPUT_FORMAT_ANY16 = 3
    SPVC_MSL_SHADER_INPUT_FORMAT_ANY32 = 4
    SPVC_MSL_SHADER_INPUT_FORMAT_INT_MAX = 2147483647
end

const spvc_msl_shader_input_format = spvc_msl_shader_variable_format

const spvc_msl_vertex_format = spvc_msl_shader_variable_format

struct spvc_msl_vertex_attribute
    location::Cuint
    msl_buffer::Cuint
    msl_offset::Cuint
    msl_stride::Cuint
    per_instance::spvc_bool
    format::spvc_msl_vertex_format
    builtin::SpvBuiltIn
end

function spvc_msl_vertex_attribute_init(attr)
    ccall((:spvc_msl_vertex_attribute_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_vertex_attribute},), attr)
end

struct spvc_msl_shader_interface_var
    location::Cuint
    format::spvc_msl_vertex_format
    builtin::SpvBuiltIn
    vecsize::Cuint
end

const spvc_msl_shader_input = spvc_msl_shader_interface_var

function spvc_msl_shader_interface_var_init(var)
    ccall((:spvc_msl_shader_interface_var_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_shader_interface_var},), var)
end

function spvc_msl_shader_input_init(input)
    ccall((:spvc_msl_shader_input_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_shader_input},), input)
end

struct spvc_msl_resource_binding
    stage::SpvExecutionModel
    desc_set::Cuint
    binding::Cuint
    msl_buffer::Cuint
    msl_texture::Cuint
    msl_sampler::Cuint
end

function spvc_msl_resource_binding_init(binding)
    ccall((:spvc_msl_resource_binding_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_resource_binding},), binding)
end

function spvc_msl_get_aux_buffer_struct_version()
    ccall((:spvc_msl_get_aux_buffer_struct_version, spirv_cross_c_shared), Cuint, ())
end

@cenum spvc_msl_sampler_coord::UInt32 begin
    SPVC_MSL_SAMPLER_COORD_NORMALIZED = 0
    SPVC_MSL_SAMPLER_COORD_PIXEL = 1
    SPVC_MSL_SAMPLER_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_filter::UInt32 begin
    SPVC_MSL_SAMPLER_FILTER_NEAREST = 0
    SPVC_MSL_SAMPLER_FILTER_LINEAR = 1
    SPVC_MSL_SAMPLER_FILTER_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_mip_filter::UInt32 begin
    SPVC_MSL_SAMPLER_MIP_FILTER_NONE = 0
    SPVC_MSL_SAMPLER_MIP_FILTER_NEAREST = 1
    SPVC_MSL_SAMPLER_MIP_FILTER_LINEAR = 2
    SPVC_MSL_SAMPLER_MIP_FILTER_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_address::UInt32 begin
    SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_ZERO = 0
    SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_EDGE = 1
    SPVC_MSL_SAMPLER_ADDRESS_CLAMP_TO_BORDER = 2
    SPVC_MSL_SAMPLER_ADDRESS_REPEAT = 3
    SPVC_MSL_SAMPLER_ADDRESS_MIRRORED_REPEAT = 4
    SPVC_MSL_SAMPLER_ADDRESS_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_compare_func::UInt32 begin
    SPVC_MSL_SAMPLER_COMPARE_FUNC_NEVER = 0
    SPVC_MSL_SAMPLER_COMPARE_FUNC_LESS = 1
    SPVC_MSL_SAMPLER_COMPARE_FUNC_LESS_EQUAL = 2
    SPVC_MSL_SAMPLER_COMPARE_FUNC_GREATER = 3
    SPVC_MSL_SAMPLER_COMPARE_FUNC_GREATER_EQUAL = 4
    SPVC_MSL_SAMPLER_COMPARE_FUNC_EQUAL = 5
    SPVC_MSL_SAMPLER_COMPARE_FUNC_NOT_EQUAL = 6
    SPVC_MSL_SAMPLER_COMPARE_FUNC_ALWAYS = 7
    SPVC_MSL_SAMPLER_COMPARE_FUNC_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_border_color::UInt32 begin
    SPVC_MSL_SAMPLER_BORDER_COLOR_TRANSPARENT_BLACK = 0
    SPVC_MSL_SAMPLER_BORDER_COLOR_OPAQUE_BLACK = 1
    SPVC_MSL_SAMPLER_BORDER_COLOR_OPAQUE_WHITE = 2
    SPVC_MSL_SAMPLER_BORDER_COLOR_INT_MAX = 2147483647
end

@cenum spvc_msl_format_resolution::UInt32 begin
    SPVC_MSL_FORMAT_RESOLUTION_444 = 0
    SPVC_MSL_FORMAT_RESOLUTION_422 = 1
    SPVC_MSL_FORMAT_RESOLUTION_420 = 2
    SPVC_MSL_FORMAT_RESOLUTION_INT_MAX = 2147483647
end

@cenum spvc_msl_chroma_location::UInt32 begin
    SPVC_MSL_CHROMA_LOCATION_COSITED_EVEN = 0
    SPVC_MSL_CHROMA_LOCATION_MIDPOINT = 1
    SPVC_MSL_CHROMA_LOCATION_INT_MAX = 2147483647
end

@cenum spvc_msl_component_swizzle::UInt32 begin
    SPVC_MSL_COMPONENT_SWIZZLE_IDENTITY = 0
    SPVC_MSL_COMPONENT_SWIZZLE_ZERO = 1
    SPVC_MSL_COMPONENT_SWIZZLE_ONE = 2
    SPVC_MSL_COMPONENT_SWIZZLE_R = 3
    SPVC_MSL_COMPONENT_SWIZZLE_G = 4
    SPVC_MSL_COMPONENT_SWIZZLE_B = 5
    SPVC_MSL_COMPONENT_SWIZZLE_A = 6
    SPVC_MSL_COMPONENT_SWIZZLE_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_ycbcr_model_conversion::UInt32 begin
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY = 0
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY = 1
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_709 = 2
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_601 = 3
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_BT_2020 = 4
    SPVC_MSL_SAMPLER_YCBCR_MODEL_CONVERSION_INT_MAX = 2147483647
end

@cenum spvc_msl_sampler_ycbcr_range::UInt32 begin
    SPVC_MSL_SAMPLER_YCBCR_RANGE_ITU_FULL = 0
    SPVC_MSL_SAMPLER_YCBCR_RANGE_ITU_NARROW = 1
    SPVC_MSL_SAMPLER_YCBCR_RANGE_INT_MAX = 2147483647
end

struct spvc_msl_constexpr_sampler
    coord::spvc_msl_sampler_coord
    min_filter::spvc_msl_sampler_filter
    mag_filter::spvc_msl_sampler_filter
    mip_filter::spvc_msl_sampler_mip_filter
    s_address::spvc_msl_sampler_address
    t_address::spvc_msl_sampler_address
    r_address::spvc_msl_sampler_address
    compare_func::spvc_msl_sampler_compare_func
    border_color::spvc_msl_sampler_border_color
    lod_clamp_min::Cfloat
    lod_clamp_max::Cfloat
    max_anisotropy::Cint
    compare_enable::spvc_bool
    lod_clamp_enable::spvc_bool
    anisotropy_enable::spvc_bool
end

function spvc_msl_constexpr_sampler_init(sampler)
    ccall((:spvc_msl_constexpr_sampler_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_constexpr_sampler},), sampler)
end

struct spvc_msl_sampler_ycbcr_conversion
    planes::Cuint
    resolution::spvc_msl_format_resolution
    chroma_filter::spvc_msl_sampler_filter
    x_chroma_offset::spvc_msl_chroma_location
    y_chroma_offset::spvc_msl_chroma_location
    swizzle::NTuple{4, spvc_msl_component_swizzle}
    ycbcr_model::spvc_msl_sampler_ycbcr_model_conversion
    ycbcr_range::spvc_msl_sampler_ycbcr_range
    bpc::Cuint
end

function spvc_msl_sampler_ycbcr_conversion_init(conv)
    ccall((:spvc_msl_sampler_ycbcr_conversion_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_msl_sampler_ycbcr_conversion},), conv)
end

@cenum spvc_hlsl_binding_flag_bits::UInt32 begin
    SPVC_HLSL_BINDING_AUTO_NONE_BIT = 0
    SPVC_HLSL_BINDING_AUTO_PUSH_CONSTANT_BIT = 1
    SPVC_HLSL_BINDING_AUTO_CBV_BIT = 2
    SPVC_HLSL_BINDING_AUTO_SRV_BIT = 4
    SPVC_HLSL_BINDING_AUTO_UAV_BIT = 8
    SPVC_HLSL_BINDING_AUTO_SAMPLER_BIT = 16
    SPVC_HLSL_BINDING_AUTO_ALL = 2147483647
end

const spvc_hlsl_binding_flags = Cuint

struct spvc_hlsl_resource_binding_mapping
    register_space::Cuint
    register_binding::Cuint
end

struct spvc_hlsl_resource_binding
    stage::SpvExecutionModel
    desc_set::Cuint
    binding::Cuint
    cbv::spvc_hlsl_resource_binding_mapping
    uav::spvc_hlsl_resource_binding_mapping
    srv::spvc_hlsl_resource_binding_mapping
    sampler::spvc_hlsl_resource_binding_mapping
end

function spvc_hlsl_resource_binding_init(binding)
    ccall((:spvc_hlsl_resource_binding_init, spirv_cross_c_shared), Cvoid, (Ptr{spvc_hlsl_resource_binding},), binding)
end

@cenum spvc_compiler_option::UInt32 begin
    SPVC_COMPILER_OPTION_UNKNOWN = 0
    SPVC_COMPILER_OPTION_FORCE_TEMPORARY = 16777217
    SPVC_COMPILER_OPTION_FLATTEN_MULTIDIMENSIONAL_ARRAYS = 16777218
    SPVC_COMPILER_OPTION_FIXUP_DEPTH_CONVENTION = 16777219
    SPVC_COMPILER_OPTION_FLIP_VERTEX_Y = 16777220
    SPVC_COMPILER_OPTION_GLSL_SUPPORT_NONZERO_BASE_INSTANCE = 33554437
    SPVC_COMPILER_OPTION_GLSL_SEPARATE_SHADER_OBJECTS = 33554438
    SPVC_COMPILER_OPTION_GLSL_ENABLE_420PACK_EXTENSION = 33554439
    SPVC_COMPILER_OPTION_GLSL_VERSION = 33554440
    SPVC_COMPILER_OPTION_GLSL_ES = 33554441
    SPVC_COMPILER_OPTION_GLSL_VULKAN_SEMANTICS = 33554442
    SPVC_COMPILER_OPTION_GLSL_ES_DEFAULT_FLOAT_PRECISION_HIGHP = 33554443
    SPVC_COMPILER_OPTION_GLSL_ES_DEFAULT_INT_PRECISION_HIGHP = 33554444
    SPVC_COMPILER_OPTION_HLSL_SHADER_MODEL = 67108877
    SPVC_COMPILER_OPTION_HLSL_POINT_SIZE_COMPAT = 67108878
    SPVC_COMPILER_OPTION_HLSL_POINT_COORD_COMPAT = 67108879
    SPVC_COMPILER_OPTION_HLSL_SUPPORT_NONZERO_BASE_VERTEX_BASE_INSTANCE = 67108880
    SPVC_COMPILER_OPTION_MSL_VERSION = 134217745
    SPVC_COMPILER_OPTION_MSL_TEXEL_BUFFER_TEXTURE_WIDTH = 134217746
    SPVC_COMPILER_OPTION_MSL_AUX_BUFFER_INDEX = 134217747
    SPVC_COMPILER_OPTION_MSL_SWIZZLE_BUFFER_INDEX = 134217747
    SPVC_COMPILER_OPTION_MSL_INDIRECT_PARAMS_BUFFER_INDEX = 134217748
    SPVC_COMPILER_OPTION_MSL_SHADER_OUTPUT_BUFFER_INDEX = 134217749
    SPVC_COMPILER_OPTION_MSL_SHADER_PATCH_OUTPUT_BUFFER_INDEX = 134217750
    SPVC_COMPILER_OPTION_MSL_SHADER_TESS_FACTOR_OUTPUT_BUFFER_INDEX = 134217751
    SPVC_COMPILER_OPTION_MSL_SHADER_INPUT_WORKGROUP_INDEX = 134217752
    SPVC_COMPILER_OPTION_MSL_ENABLE_POINT_SIZE_BUILTIN = 134217753
    SPVC_COMPILER_OPTION_MSL_DISABLE_RASTERIZATION = 134217754
    SPVC_COMPILER_OPTION_MSL_CAPTURE_OUTPUT_TO_BUFFER = 134217755
    SPVC_COMPILER_OPTION_MSL_SWIZZLE_TEXTURE_SAMPLES = 134217756
    SPVC_COMPILER_OPTION_MSL_PAD_FRAGMENT_OUTPUT_COMPONENTS = 134217757
    SPVC_COMPILER_OPTION_MSL_TESS_DOMAIN_ORIGIN_LOWER_LEFT = 134217758
    SPVC_COMPILER_OPTION_MSL_PLATFORM = 134217759
    SPVC_COMPILER_OPTION_MSL_ARGUMENT_BUFFERS = 134217760
    SPVC_COMPILER_OPTION_GLSL_EMIT_PUSH_CONSTANT_AS_UNIFORM_BUFFER = 33554465
    SPVC_COMPILER_OPTION_MSL_TEXTURE_BUFFER_NATIVE = 134217762
    SPVC_COMPILER_OPTION_GLSL_EMIT_UNIFORM_BUFFER_AS_PLAIN_UNIFORMS = 33554467
    SPVC_COMPILER_OPTION_MSL_BUFFER_SIZE_BUFFER_INDEX = 134217764
    SPVC_COMPILER_OPTION_EMIT_LINE_DIRECTIVES = 16777253
    SPVC_COMPILER_OPTION_MSL_MULTIVIEW = 134217766
    SPVC_COMPILER_OPTION_MSL_VIEW_MASK_BUFFER_INDEX = 134217767
    SPVC_COMPILER_OPTION_MSL_DEVICE_INDEX = 134217768
    SPVC_COMPILER_OPTION_MSL_VIEW_INDEX_FROM_DEVICE_INDEX = 134217769
    SPVC_COMPILER_OPTION_MSL_DISPATCH_BASE = 134217770
    SPVC_COMPILER_OPTION_MSL_DYNAMIC_OFFSETS_BUFFER_INDEX = 134217771
    SPVC_COMPILER_OPTION_MSL_TEXTURE_1D_AS_2D = 134217772
    SPVC_COMPILER_OPTION_MSL_ENABLE_BASE_INDEX_ZERO = 134217773
    SPVC_COMPILER_OPTION_MSL_IOS_FRAMEBUFFER_FETCH_SUBPASS = 134217774
    SPVC_COMPILER_OPTION_MSL_FRAMEBUFFER_FETCH_SUBPASS = 134217774
    SPVC_COMPILER_OPTION_MSL_INVARIANT_FP_MATH = 134217775
    SPVC_COMPILER_OPTION_MSL_EMULATE_CUBEMAP_ARRAY = 134217776
    SPVC_COMPILER_OPTION_MSL_ENABLE_DECORATION_BINDING = 134217777
    SPVC_COMPILER_OPTION_MSL_FORCE_ACTIVE_ARGUMENT_BUFFER_RESOURCES = 134217778
    SPVC_COMPILER_OPTION_MSL_FORCE_NATIVE_ARRAYS = 134217779
    SPVC_COMPILER_OPTION_ENABLE_STORAGE_IMAGE_QUALIFIER_DEDUCTION = 16777268
    SPVC_COMPILER_OPTION_HLSL_FORCE_STORAGE_BUFFER_AS_UAV = 67108917
    SPVC_COMPILER_OPTION_FORCE_ZERO_INITIALIZED_VARIABLES = 16777270
    SPVC_COMPILER_OPTION_HLSL_NONWRITABLE_UAV_TEXTURE_AS_SRV = 67108919
    SPVC_COMPILER_OPTION_MSL_ENABLE_FRAG_OUTPUT_MASK = 134217784
    SPVC_COMPILER_OPTION_MSL_ENABLE_FRAG_DEPTH_BUILTIN = 134217785
    SPVC_COMPILER_OPTION_MSL_ENABLE_FRAG_STENCIL_REF_BUILTIN = 134217786
    SPVC_COMPILER_OPTION_MSL_ENABLE_CLIP_DISTANCE_USER_VARYING = 134217787
    SPVC_COMPILER_OPTION_HLSL_ENABLE_16BIT_TYPES = 67108924
    SPVC_COMPILER_OPTION_MSL_MULTI_PATCH_WORKGROUP = 134217789
    SPVC_COMPILER_OPTION_MSL_SHADER_INPUT_BUFFER_INDEX = 134217790
    SPVC_COMPILER_OPTION_MSL_SHADER_INDEX_BUFFER_INDEX = 134217791
    SPVC_COMPILER_OPTION_MSL_VERTEX_FOR_TESSELLATION = 134217792
    SPVC_COMPILER_OPTION_MSL_VERTEX_INDEX_TYPE = 134217793
    SPVC_COMPILER_OPTION_GLSL_FORCE_FLATTENED_IO_BLOCKS = 33554498
    SPVC_COMPILER_OPTION_MSL_MULTIVIEW_LAYERED_RENDERING = 134217795
    SPVC_COMPILER_OPTION_MSL_ARRAYED_SUBPASS_INPUT = 134217796
    SPVC_COMPILER_OPTION_MSL_R32UI_LINEAR_TEXTURE_ALIGNMENT = 134217797
    SPVC_COMPILER_OPTION_MSL_R32UI_ALIGNMENT_CONSTANT_ID = 134217798
    SPVC_COMPILER_OPTION_HLSL_FLATTEN_MATRIX_VERTEX_INPUT_SEMANTICS = 67108935
    SPVC_COMPILER_OPTION_MSL_IOS_USE_SIMDGROUP_FUNCTIONS = 134217800
    SPVC_COMPILER_OPTION_MSL_EMULATE_SUBGROUPS = 134217801
    SPVC_COMPILER_OPTION_MSL_FIXED_SUBGROUP_SIZE = 134217802
    SPVC_COMPILER_OPTION_MSL_FORCE_SAMPLE_RATE_SHADING = 134217803
    SPVC_COMPILER_OPTION_MSL_IOS_SUPPORT_BASE_VERTEX_INSTANCE = 134217804
    SPVC_COMPILER_OPTION_GLSL_OVR_MULTIVIEW_VIEW_COUNT = 33554509
    SPVC_COMPILER_OPTION_RELAX_NAN_CHECKS = 16777294
    SPVC_COMPILER_OPTION_INT_MAX = 2147483647
end

function spvc_context_create(context)
    ccall((:spvc_context_create, spirv_cross_c_shared), spvc_result, (Ptr{spvc_context},), context)
end

function spvc_context_destroy(context)
    ccall((:spvc_context_destroy, spirv_cross_c_shared), Cvoid, (spvc_context,), context)
end

function spvc_context_release_allocations(context)
    ccall((:spvc_context_release_allocations, spirv_cross_c_shared), Cvoid, (spvc_context,), context)
end

function spvc_context_get_last_error_string(context)
    ccall((:spvc_context_get_last_error_string, spirv_cross_c_shared), Ptr{Cchar}, (spvc_context,), context)
end

# typedef void ( * spvc_error_callback ) ( void * userdata , const char * error )
const spvc_error_callback = Ptr{Cvoid}

function spvc_context_set_error_callback(context, cb, userdata)
    ccall((:spvc_context_set_error_callback, spirv_cross_c_shared), Cvoid, (spvc_context, spvc_error_callback, Ptr{Cvoid}), context, cb, userdata)
end

function spvc_context_parse_spirv(context, spirv, word_count, parsed_ir)
    ccall((:spvc_context_parse_spirv, spirv_cross_c_shared), spvc_result, (spvc_context, Ptr{SpvId}, Csize_t, Ptr{spvc_parsed_ir}), context, spirv, word_count, parsed_ir)
end

function spvc_context_create_compiler(context, backend, parsed_ir, mode, compiler)
    ccall((:spvc_context_create_compiler, spirv_cross_c_shared), spvc_result, (spvc_context, spvc_backend, spvc_parsed_ir, spvc_capture_mode, Ptr{spvc_compiler}), context, backend, parsed_ir, mode, compiler)
end

function spvc_compiler_get_current_id_bound(compiler)
    ccall((:spvc_compiler_get_current_id_bound, spirv_cross_c_shared), Cuint, (spvc_compiler,), compiler)
end

function spvc_compiler_create_compiler_options(compiler, options)
    ccall((:spvc_compiler_create_compiler_options, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_compiler_options}), compiler, options)
end

function spvc_compiler_options_set_bool(options, option, value)
    ccall((:spvc_compiler_options_set_bool, spirv_cross_c_shared), spvc_result, (spvc_compiler_options, spvc_compiler_option, spvc_bool), options, option, value)
end

function spvc_compiler_options_set_uint(options, option, value)
    ccall((:spvc_compiler_options_set_uint, spirv_cross_c_shared), spvc_result, (spvc_compiler_options, spvc_compiler_option, Cuint), options, option, value)
end

function spvc_compiler_install_compiler_options(compiler, options)
    ccall((:spvc_compiler_install_compiler_options, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_compiler_options), compiler, options)
end

function spvc_compiler_compile(compiler, source)
    ccall((:spvc_compiler_compile, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{Cchar}}), compiler, source)
end

function spvc_compiler_add_header_line(compiler, line)
    ccall((:spvc_compiler_add_header_line, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Cchar}), compiler, line)
end

function spvc_compiler_require_extension(compiler, ext)
    ccall((:spvc_compiler_require_extension, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Cchar}), compiler, ext)
end

function spvc_compiler_flatten_buffer_block(compiler, id)
    ccall((:spvc_compiler_flatten_buffer_block, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_variable_is_depth_or_compare(compiler, id)
    ccall((:spvc_compiler_variable_is_depth_or_compare, spirv_cross_c_shared), spvc_bool, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_mask_stage_output_by_location(compiler, location, component)
    ccall((:spvc_compiler_mask_stage_output_by_location, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint), compiler, location, component)
end

function spvc_compiler_mask_stage_output_by_builtin(compiler, builtin)
    ccall((:spvc_compiler_mask_stage_output_by_builtin, spirv_cross_c_shared), spvc_result, (spvc_compiler, SpvBuiltIn), compiler, builtin)
end

function spvc_compiler_hlsl_set_root_constants_layout(compiler, constant_info, count)
    ccall((:spvc_compiler_hlsl_set_root_constants_layout, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_hlsl_root_constants}, Csize_t), compiler, constant_info, count)
end

function spvc_compiler_hlsl_add_vertex_attribute_remap(compiler, remap, remaps)
    ccall((:spvc_compiler_hlsl_add_vertex_attribute_remap, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_hlsl_vertex_attribute_remap}, Csize_t), compiler, remap, remaps)
end

function spvc_compiler_hlsl_remap_num_workgroups_builtin(compiler)
    ccall((:spvc_compiler_hlsl_remap_num_workgroups_builtin, spirv_cross_c_shared), spvc_variable_id, (spvc_compiler,), compiler)
end

function spvc_compiler_hlsl_set_resource_binding_flags(compiler, flags)
    ccall((:spvc_compiler_hlsl_set_resource_binding_flags, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_hlsl_binding_flags), compiler, flags)
end

function spvc_compiler_hlsl_add_resource_binding(compiler, binding)
    ccall((:spvc_compiler_hlsl_add_resource_binding, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_hlsl_resource_binding}), compiler, binding)
end

function spvc_compiler_hlsl_is_resource_used(compiler, model, set, binding)
    ccall((:spvc_compiler_hlsl_is_resource_used, spirv_cross_c_shared), spvc_bool, (spvc_compiler, SpvExecutionModel, Cuint, Cuint), compiler, model, set, binding)
end

function spvc_compiler_msl_is_rasterization_disabled(compiler)
    ccall((:spvc_compiler_msl_is_rasterization_disabled, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_aux_buffer(compiler)
    ccall((:spvc_compiler_msl_needs_aux_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_swizzle_buffer(compiler)
    ccall((:spvc_compiler_msl_needs_swizzle_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_buffer_size_buffer(compiler)
    ccall((:spvc_compiler_msl_needs_buffer_size_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_output_buffer(compiler)
    ccall((:spvc_compiler_msl_needs_output_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_patch_output_buffer(compiler)
    ccall((:spvc_compiler_msl_needs_patch_output_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_needs_input_threadgroup_mem(compiler)
    ccall((:spvc_compiler_msl_needs_input_threadgroup_mem, spirv_cross_c_shared), spvc_bool, (spvc_compiler,), compiler)
end

function spvc_compiler_msl_add_vertex_attribute(compiler, attrs)
    ccall((:spvc_compiler_msl_add_vertex_attribute, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_msl_vertex_attribute}), compiler, attrs)
end

function spvc_compiler_msl_add_resource_binding(compiler, binding)
    ccall((:spvc_compiler_msl_add_resource_binding, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_msl_resource_binding}), compiler, binding)
end

function spvc_compiler_msl_add_shader_input(compiler, input)
    ccall((:spvc_compiler_msl_add_shader_input, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_msl_shader_interface_var}), compiler, input)
end

function spvc_compiler_msl_add_shader_output(compiler, output)
    ccall((:spvc_compiler_msl_add_shader_output, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_msl_shader_interface_var}), compiler, output)
end

function spvc_compiler_msl_add_discrete_descriptor_set(compiler, desc_set)
    ccall((:spvc_compiler_msl_add_discrete_descriptor_set, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint), compiler, desc_set)
end

function spvc_compiler_msl_set_argument_buffer_device_address_space(compiler, desc_set, device_address)
    ccall((:spvc_compiler_msl_set_argument_buffer_device_address_space, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, spvc_bool), compiler, desc_set, device_address)
end

function spvc_compiler_msl_is_vertex_attribute_used(compiler, location)
    ccall((:spvc_compiler_msl_is_vertex_attribute_used, spirv_cross_c_shared), spvc_bool, (spvc_compiler, Cuint), compiler, location)
end

function spvc_compiler_msl_is_shader_input_used(compiler, location)
    ccall((:spvc_compiler_msl_is_shader_input_used, spirv_cross_c_shared), spvc_bool, (spvc_compiler, Cuint), compiler, location)
end

function spvc_compiler_msl_is_shader_output_used(compiler, location)
    ccall((:spvc_compiler_msl_is_shader_output_used, spirv_cross_c_shared), spvc_bool, (spvc_compiler, Cuint), compiler, location)
end

function spvc_compiler_msl_is_resource_used(compiler, model, set, binding)
    ccall((:spvc_compiler_msl_is_resource_used, spirv_cross_c_shared), spvc_bool, (spvc_compiler, SpvExecutionModel, Cuint, Cuint), compiler, model, set, binding)
end

function spvc_compiler_msl_remap_constexpr_sampler(compiler, id, sampler)
    ccall((:spvc_compiler_msl_remap_constexpr_sampler, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_variable_id, Ptr{spvc_msl_constexpr_sampler}), compiler, id, sampler)
end

function spvc_compiler_msl_remap_constexpr_sampler_by_binding(compiler, desc_set, binding, sampler)
    ccall((:spvc_compiler_msl_remap_constexpr_sampler_by_binding, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint, Ptr{spvc_msl_constexpr_sampler}), compiler, desc_set, binding, sampler)
end

function spvc_compiler_msl_remap_constexpr_sampler_ycbcr(compiler, id, sampler, conv)
    ccall((:spvc_compiler_msl_remap_constexpr_sampler_ycbcr, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_variable_id, Ptr{spvc_msl_constexpr_sampler}, Ptr{spvc_msl_sampler_ycbcr_conversion}), compiler, id, sampler, conv)
end

function spvc_compiler_msl_remap_constexpr_sampler_by_binding_ycbcr(compiler, desc_set, binding, sampler, conv)
    ccall((:spvc_compiler_msl_remap_constexpr_sampler_by_binding_ycbcr, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint, Ptr{spvc_msl_constexpr_sampler}, Ptr{spvc_msl_sampler_ycbcr_conversion}), compiler, desc_set, binding, sampler, conv)
end

function spvc_compiler_msl_set_fragment_output_components(compiler, location, components)
    ccall((:spvc_compiler_msl_set_fragment_output_components, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint), compiler, location, components)
end

function spvc_compiler_msl_get_automatic_resource_binding(compiler, id)
    ccall((:spvc_compiler_msl_get_automatic_resource_binding, spirv_cross_c_shared), Cuint, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_msl_get_automatic_resource_binding_secondary(compiler, id)
    ccall((:spvc_compiler_msl_get_automatic_resource_binding_secondary, spirv_cross_c_shared), Cuint, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_msl_add_dynamic_buffer(compiler, desc_set, binding, index)
    ccall((:spvc_compiler_msl_add_dynamic_buffer, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint, Cuint), compiler, desc_set, binding, index)
end

function spvc_compiler_msl_add_inline_uniform_block(compiler, desc_set, binding)
    ccall((:spvc_compiler_msl_add_inline_uniform_block, spirv_cross_c_shared), spvc_result, (spvc_compiler, Cuint, Cuint), compiler, desc_set, binding)
end

function spvc_compiler_msl_set_combined_sampler_suffix(compiler, suffix)
    ccall((:spvc_compiler_msl_set_combined_sampler_suffix, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Cchar}), compiler, suffix)
end

function spvc_compiler_msl_get_combined_sampler_suffix(compiler)
    ccall((:spvc_compiler_msl_get_combined_sampler_suffix, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler,), compiler)
end

function spvc_compiler_get_active_interface_variables(compiler, set)
    ccall((:spvc_compiler_get_active_interface_variables, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_set}), compiler, set)
end

function spvc_compiler_set_enabled_interface_variables(compiler, set)
    ccall((:spvc_compiler_set_enabled_interface_variables, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_set), compiler, set)
end

function spvc_compiler_create_shader_resources(compiler, resources)
    ccall((:spvc_compiler_create_shader_resources, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_resources}), compiler, resources)
end

function spvc_compiler_create_shader_resources_for_active_variables(compiler, resources, active)
    ccall((:spvc_compiler_create_shader_resources_for_active_variables, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_resources}, spvc_set), compiler, resources, active)
end

function spvc_resources_get_resource_list_for_type(resources, type, resource_list, resource_size)
    ccall((:spvc_resources_get_resource_list_for_type, spirv_cross_c_shared), spvc_result, (spvc_resources, spvc_resource_type, Ptr{Ptr{spvc_reflected_resource}}, Ptr{Csize_t}), resources, type, resource_list, resource_size)
end

function spvc_resources_get_builtin_resource_list_for_type(resources, type, resource_list, resource_size)
    ccall((:spvc_resources_get_builtin_resource_list_for_type, spirv_cross_c_shared), spvc_result, (spvc_resources, spvc_builtin_resource_type, Ptr{Ptr{spvc_reflected_builtin_resource}}, Ptr{Csize_t}), resources, type, resource_list, resource_size)
end

function spvc_compiler_set_decoration(compiler, id, decoration, argument)
    ccall((:spvc_compiler_set_decoration, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvId, SpvDecoration, Cuint), compiler, id, decoration, argument)
end

function spvc_compiler_set_decoration_string(compiler, id, decoration, argument)
    ccall((:spvc_compiler_set_decoration_string, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvId, SpvDecoration, Ptr{Cchar}), compiler, id, decoration, argument)
end

function spvc_compiler_set_name(compiler, id, argument)
    ccall((:spvc_compiler_set_name, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvId, Ptr{Cchar}), compiler, id, argument)
end

function spvc_compiler_set_member_decoration(compiler, id, member_index, decoration, argument)
    ccall((:spvc_compiler_set_member_decoration, spirv_cross_c_shared), Cvoid, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration, Cuint), compiler, id, member_index, decoration, argument)
end

function spvc_compiler_set_member_decoration_string(compiler, id, member_index, decoration, argument)
    ccall((:spvc_compiler_set_member_decoration_string, spirv_cross_c_shared), Cvoid, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration, Ptr{Cchar}), compiler, id, member_index, decoration, argument)
end

function spvc_compiler_set_member_name(compiler, id, member_index, argument)
    ccall((:spvc_compiler_set_member_name, spirv_cross_c_shared), Cvoid, (spvc_compiler, spvc_type_id, Cuint, Ptr{Cchar}), compiler, id, member_index, argument)
end

function spvc_compiler_unset_decoration(compiler, id, decoration)
    ccall((:spvc_compiler_unset_decoration, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvId, SpvDecoration), compiler, id, decoration)
end

function spvc_compiler_unset_member_decoration(compiler, id, member_index, decoration)
    ccall((:spvc_compiler_unset_member_decoration, spirv_cross_c_shared), Cvoid, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration), compiler, id, member_index, decoration)
end

function spvc_compiler_has_decoration(compiler, id, decoration)
    ccall((:spvc_compiler_has_decoration, spirv_cross_c_shared), spvc_bool, (spvc_compiler, SpvId, SpvDecoration), compiler, id, decoration)
end

function spvc_compiler_has_member_decoration(compiler, id, member_index, decoration)
    ccall((:spvc_compiler_has_member_decoration, spirv_cross_c_shared), spvc_bool, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration), compiler, id, member_index, decoration)
end

function spvc_compiler_get_name(compiler, id)
    ccall((:spvc_compiler_get_name, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, SpvId), compiler, id)
end

function spvc_compiler_get_decoration(compiler, id, decoration)
    ccall((:spvc_compiler_get_decoration, spirv_cross_c_shared), Cuint, (spvc_compiler, SpvId, SpvDecoration), compiler, id, decoration)
end

function spvc_compiler_get_decoration_string(compiler, id, decoration)
    ccall((:spvc_compiler_get_decoration_string, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, SpvId, SpvDecoration), compiler, id, decoration)
end

function spvc_compiler_get_member_decoration(compiler, id, member_index, decoration)
    ccall((:spvc_compiler_get_member_decoration, spirv_cross_c_shared), Cuint, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration), compiler, id, member_index, decoration)
end

function spvc_compiler_get_member_decoration_string(compiler, id, member_index, decoration)
    ccall((:spvc_compiler_get_member_decoration_string, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, spvc_type_id, Cuint, SpvDecoration), compiler, id, member_index, decoration)
end

function spvc_compiler_get_member_name(compiler, id, member_index)
    ccall((:spvc_compiler_get_member_name, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, spvc_type_id, Cuint), compiler, id, member_index)
end

function spvc_compiler_get_entry_points(compiler, entry_points, num_entry_points)
    ccall((:spvc_compiler_get_entry_points, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{spvc_entry_point}}, Ptr{Csize_t}), compiler, entry_points, num_entry_points)
end

function spvc_compiler_set_entry_point(compiler, name, model)
    ccall((:spvc_compiler_set_entry_point, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Cchar}, SpvExecutionModel), compiler, name, model)
end

function spvc_compiler_rename_entry_point(compiler, old_name, new_name, model)
    ccall((:spvc_compiler_rename_entry_point, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Cchar}, Ptr{Cchar}, SpvExecutionModel), compiler, old_name, new_name, model)
end

function spvc_compiler_get_cleansed_entry_point_name(compiler, name, model)
    ccall((:spvc_compiler_get_cleansed_entry_point_name, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, Ptr{Cchar}, SpvExecutionModel), compiler, name, model)
end

function spvc_compiler_set_execution_mode(compiler, mode)
    ccall((:spvc_compiler_set_execution_mode, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvExecutionMode), compiler, mode)
end

function spvc_compiler_unset_execution_mode(compiler, mode)
    ccall((:spvc_compiler_unset_execution_mode, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvExecutionMode), compiler, mode)
end

function spvc_compiler_set_execution_mode_with_arguments(compiler, mode, arg0, arg1, arg2)
    ccall((:spvc_compiler_set_execution_mode_with_arguments, spirv_cross_c_shared), Cvoid, (spvc_compiler, SpvExecutionMode, Cuint, Cuint, Cuint), compiler, mode, arg0, arg1, arg2)
end

function spvc_compiler_get_execution_modes(compiler, modes, num_modes)
    ccall((:spvc_compiler_get_execution_modes, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{SpvExecutionMode}}, Ptr{Csize_t}), compiler, modes, num_modes)
end

function spvc_compiler_get_execution_mode_argument(compiler, mode)
    ccall((:spvc_compiler_get_execution_mode_argument, spirv_cross_c_shared), Cuint, (spvc_compiler, SpvExecutionMode), compiler, mode)
end

function spvc_compiler_get_execution_mode_argument_by_index(compiler, mode, index)
    ccall((:spvc_compiler_get_execution_mode_argument_by_index, spirv_cross_c_shared), Cuint, (spvc_compiler, SpvExecutionMode, Cuint), compiler, mode, index)
end

function spvc_compiler_get_execution_model(compiler)
    ccall((:spvc_compiler_get_execution_model, spirv_cross_c_shared), SpvExecutionModel, (spvc_compiler,), compiler)
end

function spvc_compiler_update_active_builtins(compiler)
    ccall((:spvc_compiler_update_active_builtins, spirv_cross_c_shared), Cvoid, (spvc_compiler,), compiler)
end

function spvc_compiler_has_active_builtin(compiler, builtin, storage)
    ccall((:spvc_compiler_has_active_builtin, spirv_cross_c_shared), spvc_bool, (spvc_compiler, SpvBuiltIn, SpvStorageClass), compiler, builtin, storage)
end

function spvc_compiler_get_type_handle(compiler, id)
    ccall((:spvc_compiler_get_type_handle, spirv_cross_c_shared), spvc_type, (spvc_compiler, spvc_type_id), compiler, id)
end

function spvc_type_get_base_type_id(type)
    ccall((:spvc_type_get_base_type_id, spirv_cross_c_shared), spvc_type_id, (spvc_type,), type)
end

function spvc_type_get_basetype(type)
    ccall((:spvc_type_get_basetype, spirv_cross_c_shared), spvc_basetype, (spvc_type,), type)
end

function spvc_type_get_bit_width(type)
    ccall((:spvc_type_get_bit_width, spirv_cross_c_shared), Cuint, (spvc_type,), type)
end

function spvc_type_get_vector_size(type)
    ccall((:spvc_type_get_vector_size, spirv_cross_c_shared), Cuint, (spvc_type,), type)
end

function spvc_type_get_columns(type)
    ccall((:spvc_type_get_columns, spirv_cross_c_shared), Cuint, (spvc_type,), type)
end

function spvc_type_get_num_array_dimensions(type)
    ccall((:spvc_type_get_num_array_dimensions, spirv_cross_c_shared), Cuint, (spvc_type,), type)
end

function spvc_type_array_dimension_is_literal(type, dimension)
    ccall((:spvc_type_array_dimension_is_literal, spirv_cross_c_shared), spvc_bool, (spvc_type, Cuint), type, dimension)
end

function spvc_type_get_array_dimension(type, dimension)
    ccall((:spvc_type_get_array_dimension, spirv_cross_c_shared), SpvId, (spvc_type, Cuint), type, dimension)
end

function spvc_type_get_num_member_types(type)
    ccall((:spvc_type_get_num_member_types, spirv_cross_c_shared), Cuint, (spvc_type,), type)
end

function spvc_type_get_member_type(type, index)
    ccall((:spvc_type_get_member_type, spirv_cross_c_shared), spvc_type_id, (spvc_type, Cuint), type, index)
end

function spvc_type_get_storage_class(type)
    ccall((:spvc_type_get_storage_class, spirv_cross_c_shared), SpvStorageClass, (spvc_type,), type)
end

function spvc_type_get_image_sampled_type(type)
    ccall((:spvc_type_get_image_sampled_type, spirv_cross_c_shared), spvc_type_id, (spvc_type,), type)
end

function spvc_type_get_image_dimension(type)
    ccall((:spvc_type_get_image_dimension, spirv_cross_c_shared), SpvDim, (spvc_type,), type)
end

function spvc_type_get_image_is_depth(type)
    ccall((:spvc_type_get_image_is_depth, spirv_cross_c_shared), spvc_bool, (spvc_type,), type)
end

function spvc_type_get_image_arrayed(type)
    ccall((:spvc_type_get_image_arrayed, spirv_cross_c_shared), spvc_bool, (spvc_type,), type)
end

function spvc_type_get_image_multisampled(type)
    ccall((:spvc_type_get_image_multisampled, spirv_cross_c_shared), spvc_bool, (spvc_type,), type)
end

function spvc_type_get_image_is_storage(type)
    ccall((:spvc_type_get_image_is_storage, spirv_cross_c_shared), spvc_bool, (spvc_type,), type)
end

function spvc_type_get_image_storage_format(type)
    ccall((:spvc_type_get_image_storage_format, spirv_cross_c_shared), SpvImageFormat, (spvc_type,), type)
end

function spvc_type_get_image_access_qualifier(type)
    ccall((:spvc_type_get_image_access_qualifier, spirv_cross_c_shared), SpvAccessQualifier, (spvc_type,), type)
end

function spvc_compiler_get_declared_struct_size(compiler, struct_type, size)
    ccall((:spvc_compiler_get_declared_struct_size, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Ptr{Csize_t}), compiler, struct_type, size)
end

function spvc_compiler_get_declared_struct_size_runtime_array(compiler, struct_type, array_size, size)
    ccall((:spvc_compiler_get_declared_struct_size_runtime_array, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Csize_t, Ptr{Csize_t}), compiler, struct_type, array_size, size)
end

function spvc_compiler_get_declared_struct_member_size(compiler, type, index, size)
    ccall((:spvc_compiler_get_declared_struct_member_size, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Cuint, Ptr{Csize_t}), compiler, type, index, size)
end

function spvc_compiler_type_struct_member_offset(compiler, type, index, offset)
    ccall((:spvc_compiler_type_struct_member_offset, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Cuint, Ptr{Cuint}), compiler, type, index, offset)
end

function spvc_compiler_type_struct_member_array_stride(compiler, type, index, stride)
    ccall((:spvc_compiler_type_struct_member_array_stride, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Cuint, Ptr{Cuint}), compiler, type, index, stride)
end

function spvc_compiler_type_struct_member_matrix_stride(compiler, type, index, stride)
    ccall((:spvc_compiler_type_struct_member_matrix_stride, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_type, Cuint, Ptr{Cuint}), compiler, type, index, stride)
end

function spvc_compiler_build_dummy_sampler_for_combined_images(compiler, id)
    ccall((:spvc_compiler_build_dummy_sampler_for_combined_images, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{spvc_variable_id}), compiler, id)
end

function spvc_compiler_build_combined_image_samplers(compiler)
    ccall((:spvc_compiler_build_combined_image_samplers, spirv_cross_c_shared), spvc_result, (spvc_compiler,), compiler)
end

function spvc_compiler_get_combined_image_samplers(compiler, samplers, num_samplers)
    ccall((:spvc_compiler_get_combined_image_samplers, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{spvc_combined_image_sampler}}, Ptr{Csize_t}), compiler, samplers, num_samplers)
end

function spvc_compiler_get_specialization_constants(compiler, constants, num_constants)
    ccall((:spvc_compiler_get_specialization_constants, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{spvc_specialization_constant}}, Ptr{Csize_t}), compiler, constants, num_constants)
end

function spvc_compiler_get_constant_handle(compiler, id)
    ccall((:spvc_compiler_get_constant_handle, spirv_cross_c_shared), spvc_constant, (spvc_compiler, spvc_constant_id), compiler, id)
end

function spvc_compiler_get_work_group_size_specialization_constants(compiler, x, y, z)
    ccall((:spvc_compiler_get_work_group_size_specialization_constants, spirv_cross_c_shared), spvc_constant_id, (spvc_compiler, Ptr{spvc_specialization_constant}, Ptr{spvc_specialization_constant}, Ptr{spvc_specialization_constant}), compiler, x, y, z)
end

function spvc_compiler_get_active_buffer_ranges(compiler, id, ranges, num_ranges)
    ccall((:spvc_compiler_get_active_buffer_ranges, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_variable_id, Ptr{Ptr{spvc_buffer_range}}, Ptr{Csize_t}), compiler, id, ranges, num_ranges)
end

function spvc_constant_get_scalar_fp16(constant, column, row)
    ccall((:spvc_constant_get_scalar_fp16, spirv_cross_c_shared), Cfloat, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_fp32(constant, column, row)
    ccall((:spvc_constant_get_scalar_fp32, spirv_cross_c_shared), Cfloat, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_fp64(constant, column, row)
    ccall((:spvc_constant_get_scalar_fp64, spirv_cross_c_shared), Cdouble, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_u32(constant, column, row)
    ccall((:spvc_constant_get_scalar_u32, spirv_cross_c_shared), Cuint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_i32(constant, column, row)
    ccall((:spvc_constant_get_scalar_i32, spirv_cross_c_shared), Cint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_u16(constant, column, row)
    ccall((:spvc_constant_get_scalar_u16, spirv_cross_c_shared), Cuint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_i16(constant, column, row)
    ccall((:spvc_constant_get_scalar_i16, spirv_cross_c_shared), Cint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_u8(constant, column, row)
    ccall((:spvc_constant_get_scalar_u8, spirv_cross_c_shared), Cuint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_scalar_i8(constant, column, row)
    ccall((:spvc_constant_get_scalar_i8, spirv_cross_c_shared), Cint, (spvc_constant, Cuint, Cuint), constant, column, row)
end

function spvc_constant_get_subconstants(constant, constituents, count)
    ccall((:spvc_constant_get_subconstants, spirv_cross_c_shared), Cvoid, (spvc_constant, Ptr{Ptr{spvc_constant_id}}, Ptr{Csize_t}), constant, constituents, count)
end

function spvc_constant_get_type(constant)
    ccall((:spvc_constant_get_type, spirv_cross_c_shared), spvc_type_id, (spvc_constant,), constant)
end

function spvc_compiler_get_binary_offset_for_decoration(compiler, id, decoration, word_offset)
    ccall((:spvc_compiler_get_binary_offset_for_decoration, spirv_cross_c_shared), spvc_bool, (spvc_compiler, spvc_variable_id, SpvDecoration, Ptr{Cuint}), compiler, id, decoration, word_offset)
end

function spvc_compiler_buffer_is_hlsl_counter_buffer(compiler, id)
    ccall((:spvc_compiler_buffer_is_hlsl_counter_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_buffer_get_hlsl_counter_buffer(compiler, id, counter_id)
    ccall((:spvc_compiler_buffer_get_hlsl_counter_buffer, spirv_cross_c_shared), spvc_bool, (spvc_compiler, spvc_variable_id, Ptr{spvc_variable_id}), compiler, id, counter_id)
end

function spvc_compiler_get_declared_capabilities(compiler, capabilities, num_capabilities)
    ccall((:spvc_compiler_get_declared_capabilities, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{SpvCapability}}, Ptr{Csize_t}), compiler, capabilities, num_capabilities)
end

function spvc_compiler_get_declared_extensions(compiler, extensions, num_extensions)
    ccall((:spvc_compiler_get_declared_extensions, spirv_cross_c_shared), spvc_result, (spvc_compiler, Ptr{Ptr{Ptr{Cchar}}}, Ptr{Csize_t}), compiler, extensions, num_extensions)
end

function spvc_compiler_get_remapped_declared_block_name(compiler, id)
    ccall((:spvc_compiler_get_remapped_declared_block_name, spirv_cross_c_shared), Ptr{Cchar}, (spvc_compiler, spvc_variable_id), compiler, id)
end

function spvc_compiler_get_buffer_block_decorations(compiler, id, decorations, num_decorations)
    ccall((:spvc_compiler_get_buffer_block_decorations, spirv_cross_c_shared), spvc_result, (spvc_compiler, spvc_variable_id, Ptr{Ptr{SpvDecoration}}, Ptr{Csize_t}), compiler, id, decorations, num_decorations)
end

const SPV_VERSION = 0x00010600

const SPV_REVISION = 1

const SPVC_C_API_VERSION_MAJOR = 0

const SPVC_C_API_VERSION_MINOR = 50

const SPVC_C_API_VERSION_PATCH = 0

const SPVC_TRUE = spvc_bool(1)

const SPVC_FALSE = spvc_bool(0)

const SPVC_COMPILER_OPTION_COMMON_BIT = 0x01000000

const SPVC_COMPILER_OPTION_GLSL_BIT = 0x02000000

const SPVC_COMPILER_OPTION_HLSL_BIT = 0x04000000

const SPVC_COMPILER_OPTION_MSL_BIT = 0x08000000

const SPVC_COMPILER_OPTION_LANG_BITS = 0x0f000000

const SPVC_COMPILER_OPTION_ENUM_BITS = 0x00ffffff

const SPVC_MSL_PUSH_CONSTANT_DESC_SET = ~(Cuint(0))

const SPVC_MSL_PUSH_CONSTANT_BINDING = 0

const SPVC_MSL_SWIZZLE_BUFFER_BINDING = ~(Cuint(1))

const SPVC_MSL_BUFFER_SIZE_BUFFER_BINDING = ~(Cuint(2))

const SPVC_MSL_ARGUMENT_BUFFER_BINDING = ~(Cuint(3))

const SPVC_MSL_AUX_BUFFER_STRUCT_VERSION = 1

const SPVC_HLSL_PUSH_CONSTANT_DESC_SET = ~(Cuint(0))

const SPVC_HLSL_PUSH_CONSTANT_BINDING = 0

end # module
