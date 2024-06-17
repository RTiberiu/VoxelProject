// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealTest/Private/CustomChunkWorld.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeCustomChunkWorld() {}

// Begin Cross Module References
COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
UNREALTEST_API UClass* Z_Construct_UClass_ACustomChunkWorld();
UNREALTEST_API UClass* Z_Construct_UClass_ACustomChunkWorld_NoRegister();
UPackage* Z_Construct_UPackage__Script_UnrealTest();
// End Cross Module References

// Begin Class ACustomChunkWorld
void ACustomChunkWorld::StaticRegisterNativesACustomChunkWorld()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(ACustomChunkWorld);
UClass* Z_Construct_UClass_ACustomChunkWorld_NoRegister()
{
	return ACustomChunkWorld::StaticClass();
}
struct Z_Construct_UClass_ACustomChunkWorld_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "CustomChunkWorld.h" },
		{ "ModuleRelativePath", "Private/CustomChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_chunk_MetaData[] = {
		{ "Category", "Custom Chunk World" },
		{ "ModuleRelativePath", "Private/CustomChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_drawDistance_MetaData[] = {
		{ "Category", "Custom Chunk World" },
		{ "ModuleRelativePath", "Private/CustomChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_chunkSize_MetaData[] = {
		{ "Category", "Custom Chunk World" },
		{ "ModuleRelativePath", "Private/CustomChunkWorld.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FClassPropertyParams NewProp_chunk;
	static const UECodeGen_Private::FIntPropertyParams NewProp_drawDistance;
	static const UECodeGen_Private::FIntPropertyParams NewProp_chunkSize;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ACustomChunkWorld>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FClassPropertyParams Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_chunk = { "chunk", nullptr, (EPropertyFlags)0x0014000000000001, UECodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACustomChunkWorld, chunk), Z_Construct_UClass_UClass, Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_chunk_MetaData), NewProp_chunk_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_drawDistance = { "drawDistance", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACustomChunkWorld, drawDistance), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_drawDistance_MetaData), NewProp_drawDistance_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_chunkSize = { "chunkSize", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ACustomChunkWorld, chunkSize), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_chunkSize_MetaData), NewProp_chunkSize_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ACustomChunkWorld_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_chunk,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_drawDistance,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ACustomChunkWorld_Statics::NewProp_chunkSize,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunkWorld_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_ACustomChunkWorld_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_UnrealTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunkWorld_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_ACustomChunkWorld_Statics::ClassParams = {
	&ACustomChunkWorld::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_ACustomChunkWorld_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunkWorld_Statics::PropPointers),
	0,
	0x008000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_ACustomChunkWorld_Statics::Class_MetaDataParams), Z_Construct_UClass_ACustomChunkWorld_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_ACustomChunkWorld()
{
	if (!Z_Registration_Info_UClass_ACustomChunkWorld.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_ACustomChunkWorld.OuterSingleton, Z_Construct_UClass_ACustomChunkWorld_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_ACustomChunkWorld.OuterSingleton;
}
template<> UNREALTEST_API UClass* StaticClass<ACustomChunkWorld>()
{
	return ACustomChunkWorld::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(ACustomChunkWorld);
ACustomChunkWorld::~ACustomChunkWorld() {}
// End Class ACustomChunkWorld

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunkWorld_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_ACustomChunkWorld, ACustomChunkWorld::StaticClass, TEXT("ACustomChunkWorld"), &Z_Registration_Info_UClass_ACustomChunkWorld, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(ACustomChunkWorld), 3835043373U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunkWorld_h_2006889646(TEXT("/Script/UnrealTest"),
	Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunkWorld_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_CustomChunkWorld_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
