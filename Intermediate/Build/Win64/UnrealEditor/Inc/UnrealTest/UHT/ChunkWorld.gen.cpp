// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UnrealTest/Private/ChunkWorld.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeChunkWorld() {}

// Begin Cross Module References
COREUOBJECT_API UClass* Z_Construct_UClass_UClass();
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_AActor_NoRegister();
UNREALTEST_API UClass* Z_Construct_UClass_AChunkWorld();
UNREALTEST_API UClass* Z_Construct_UClass_AChunkWorld_NoRegister();
UPackage* Z_Construct_UPackage__Script_UnrealTest();
// End Cross Module References

// Begin Class AChunkWorld
void AChunkWorld::StaticRegisterNativesAChunkWorld()
{
}
IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AChunkWorld);
UClass* Z_Construct_UClass_AChunkWorld_NoRegister()
{
	return AChunkWorld::StaticClass();
}
struct Z_Construct_UClass_AChunkWorld_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "ChunkWorld.h" },
		{ "ModuleRelativePath", "Private/ChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Chunk_MetaData[] = {
		{ "Category", "Chunk World" },
		{ "ModuleRelativePath", "Private/ChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DrawDistance_MetaData[] = {
		{ "Category", "Chunk World" },
		{ "ModuleRelativePath", "Private/ChunkWorld.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkSize_MetaData[] = {
		{ "Category", "Chunk World" },
		{ "ModuleRelativePath", "Private/ChunkWorld.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FClassPropertyParams NewProp_Chunk;
	static const UECodeGen_Private::FIntPropertyParams NewProp_DrawDistance;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkSize;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AChunkWorld>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FClassPropertyParams Z_Construct_UClass_AChunkWorld_Statics::NewProp_Chunk = { "Chunk", nullptr, (EPropertyFlags)0x0014000000000001, UECodeGen_Private::EPropertyGenFlags::Class, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AChunkWorld, Chunk), Z_Construct_UClass_UClass, Z_Construct_UClass_AActor_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Chunk_MetaData), NewProp_Chunk_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AChunkWorld_Statics::NewProp_DrawDistance = { "DrawDistance", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AChunkWorld, DrawDistance), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DrawDistance_MetaData), NewProp_DrawDistance_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AChunkWorld_Statics::NewProp_ChunkSize = { "ChunkSize", nullptr, (EPropertyFlags)0x0010000000000001, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AChunkWorld, ChunkSize), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkSize_MetaData), NewProp_ChunkSize_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AChunkWorld_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AChunkWorld_Statics::NewProp_Chunk,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AChunkWorld_Statics::NewProp_DrawDistance,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AChunkWorld_Statics::NewProp_ChunkSize,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AChunkWorld_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AChunkWorld_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_UnrealTest,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AChunkWorld_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AChunkWorld_Statics::ClassParams = {
	&AChunkWorld::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_AChunkWorld_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_AChunkWorld_Statics::PropPointers),
	0,
	0x008000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AChunkWorld_Statics::Class_MetaDataParams), Z_Construct_UClass_AChunkWorld_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AChunkWorld()
{
	if (!Z_Registration_Info_UClass_AChunkWorld.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AChunkWorld.OuterSingleton, Z_Construct_UClass_AChunkWorld_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AChunkWorld.OuterSingleton;
}
template<> UNREALTEST_API UClass* StaticClass<AChunkWorld>()
{
	return AChunkWorld::StaticClass();
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AChunkWorld);
AChunkWorld::~AChunkWorld() {}
// End Class AChunkWorld

// Begin Registration
struct Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_ChunkWorld_h_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AChunkWorld, AChunkWorld::StaticClass, TEXT("AChunkWorld"), &Z_Registration_Info_UClass_AChunkWorld, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AChunkWorld), 4172868276U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_ChunkWorld_h_2335703680(TEXT("/Script/UnrealTest"),
	Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_ChunkWorld_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_rociu_Documents_Unreal_Projects_UnrealTest_Source_UnrealTest_Private_ChunkWorld_h_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// End Registration
PRAGMA_ENABLE_DEPRECATION_WARNINGS
