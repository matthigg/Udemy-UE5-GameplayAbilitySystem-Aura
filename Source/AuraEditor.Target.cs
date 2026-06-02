// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraEditorTarget : TargetRules
{
	public AuraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );

		// Force the compiler to use VS2022
		WindowsPlatform.Compiler = WindowsCompiler.VisualStudio2022;
		
		// Disable version check, override build env for installed engine, and suppress warnings
		bOverrideBuildEnvironment = true;
		GlobalDefinitions.Add("PLATFORM_COMPILER_VERSION_CHECK_ENABLED=0");
		AdditionalCompilerArguments = "/wd4668 /wd4067";
	}
}