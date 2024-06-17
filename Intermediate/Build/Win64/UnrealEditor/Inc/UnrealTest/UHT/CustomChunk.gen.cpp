// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealTest/Private/CustomChunk.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeCustomChunk() {}

// Begin Cross Module References
ENGINE_API UClass* Z_Construct_UClass_AActor();
UNREALTEST_API UClass* Z_Construct_UClass_ACustomChunk();
UNREALTEST_API UClass* Z_Construct_UClass_ACustomChunk_NoRegister();
UPackage* Z_Construct_UPackage__Script_UnrealTest();
// End Cross Module References

// Begin Class ACustomChunk
void ACustomChunk::StaticRegisterNativesACustomChunk()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(ACustomChunk);
UClass* Z_Construct_UClass_ACustomChunk_NoRegister()
{
	return ACustomChunk::StaticClass();
}
struct Z_Construct_UClass_ACustomChunk_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "CustomChunk.h" },
		{ "ModuleRelativePath", "Private/CustomChunk.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Size_MetaData[] = {
		{ "Category", "Chunk" },
		{ "ModuleRelativePath", "Private/CustomChunk.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Scale_MetaData[] = {
		{ "Category", "Chunk" },
		{ "ModuleRelativePath", "Private/CustomChunk.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_Size;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Scale;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ACustomChunk>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_ACustomChunk_Statics::NewProp_Size = { "Size", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACustomChunk, Size), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Size_MetaData), NewProp_Size_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_ACustomChunk_Statics::NewProp_Scale = { "Scale", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACustomChunk, Scale), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Scale_MetaData), NewProp_Scale_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ACustomChunk_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACustomChunk_Statics::NewProp_Size,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACustomChunk_Statics::NewProp_Scale,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunk_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_ACustomChunk_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_UnrealTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunk_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_ACustomChunk_Statics::ClassParams = {
	&ACustomChunk::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_ACustomChunk_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunk_Statics::PropPointers),
	0,
	0x008000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunk_Statics::Class_MetaDataParams), Z_Construct_UClass_ACustomChunk_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_ACustomChunk()
{
	if (!Z_Registration_Info_UClass_ACustomChunk.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_ACustomChunk.OuterSingleton, Z_Construct_UClass_ACustomChunk_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_ACustomChunk.OuterSingleton;
}
template<> UNREALTEST_API UClass* StaticClass<ACustomChunk>()
{
	return ACustomChunk::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(ACustomChunk);
ACustomChunk::~ACustomChunk() {}
// End Class ACustomChunk

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunk_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_ACustomChunk, ACustomChunk::StaticClass, TEXT("ACustomChunk"), &Z_Registration_Info_UClass_ACustomChunk, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(ACustomChunk), 2026729678U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunk_h_2268603093(TEXT("/Script/UnrealTest"),
	Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunk_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunk_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
