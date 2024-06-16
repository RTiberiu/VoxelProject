// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeUnrealTest_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_UnrealTest;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_UnrealTest()
	{
		if (!Z_Registration_Info_UPackage__Script_UnrealTest.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/UnrealTest",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000000,
				0xAB344576,
				0x8F3CC7F8,
				METADATA_PARAMS(0, nullptr)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_UnrealTest.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_UnrealTest.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_UnrealTest(Z_Construct_UPackage__Script_UnrealTest, TEXT("/Script/UnrealTest"), Z_Registration_Info_UPackage__Script_UnrealTest, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xAB344576, 0x8F3CC7F8));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
