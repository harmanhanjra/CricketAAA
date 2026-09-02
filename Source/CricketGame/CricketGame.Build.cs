using UnrealBuildTool;

public class CricketGame : ModuleRules
{
    public CricketGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "PhysicsCore",
            "AIModule",
            "NavigationSystem",
            "UMG",
            "Slate",
            "SlateCore"
        });
    }
}
