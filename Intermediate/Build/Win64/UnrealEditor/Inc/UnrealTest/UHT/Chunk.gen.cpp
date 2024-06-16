// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealTest/Private/Chunk.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeChunk() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AActor();
UNREALTEST_API UClass* Z_Construct_UClass_AChunk();
UNREALTEST_API UClass* Z_Construct_UClass_AChunk_NoRegister();
UPackage* Z_Construct_UPackage__Script_UnrealTest();
// End Cross Module References

// Begin Class AChunk
void AChunk::StaticRegisterNativesAChunk()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AChunk);
UClass* Z_Construct_UClass_AChunk_NoRegister()
{
	return AChunk::StaticClass();
}
struct Z_Construct_UClass_AChunk_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "Chunk.h" },
		{ "ModuleRelativePath", "Private/Chunk.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_size_MetaData[] = {
		{ "Category", "Chunk" },
		{ "ModuleRelativePath", "Private/Chunk.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_scale_MetaData[] = {
		{ "Category", "Chunk" },
		{ "ModuleRelativePath", "Private/Chunk.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_size;
	static const UECodeGen_Private::FIntPropertyParams NewProp_scale;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AChunk>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AChunk_Statics::NewProp_size = { "size", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AChunk, size), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_size_MetaData), NewProp_size_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AChunk_Statics::NewProp_scale = { "scale", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AChunk, scale), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_scale_MetaData), NewProp_scale_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AChunk_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AChunk_Statics::NewProp_size,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AChunk_Statics::NewProp_scale,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AChunk_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AChunk_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_UnrealTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AChunk_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AChunk_Statics::ClassParams = {
	&AChunk::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AChunk_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AChunk_Statics::PropPointers),
	0,
	0x008000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AChunk_Statics::Class_MetaDataParams), Z_Construct_UClass_AChunk_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AChunk()
{
	if (!Z_Registration_Info_UClass_AChunk.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AChunk.OuterSingleton, Z_Construct_UClass_AChunk_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AChunk.OuterSingleton;
}
template<> UNREALTEST_API UClass* StaticClass<AChunk>()
{
	return AChunk::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AChunk);
AChunk::~AChunk() {}
// End Class AChunk

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_Chunk_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AChunk, AChunk::StaticClass, TEXT("AChunk"), &Z_Registration_Info_UClass_AChunk, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AChunk), 3148922910U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_Chunk_h_1810471753(TEXT("/Script/UnrealTest"),
	Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_Chunk_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_Chunk_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
