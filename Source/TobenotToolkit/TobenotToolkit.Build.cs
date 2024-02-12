// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

using UnrealBuildTool;

public class TobenotToolkit : ModuleRules
{
	public TobenotToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {

			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "Source/Editor/Blutility/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", 
				"UMGEditor",
				"ContentBrowser",
				"UMG",
				"UnrealEd",
				"Blutility",
				"InputCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
