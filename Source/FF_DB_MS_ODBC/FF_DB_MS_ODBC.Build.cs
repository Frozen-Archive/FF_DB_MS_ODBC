// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_DB_MS_ODBC : ModuleRules
{
	public FF_DB_MS_ODBC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
				"Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
			}
			);

        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
