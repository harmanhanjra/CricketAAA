#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Match/ScoreManager.h"
#include "Rules/MatchRulesManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FScoreOversTest, "CricketGame.Rules.ScoreOvers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FScoreOversTest::RunTest(const FString& Parameters)
{
    UScoreManager* Score = NewObject<UScoreManager>();
    Score->Reset(2);
    Score->AddRuns(4);
    for (int32 i = 0; i < 7; ++i) Score->RecordLegalBall();

    TestEqual(TEXT("Runs"), Score->Runs, 4);
    TestEqual(TEXT("Overs notation"), Score->GetOversText(), FString(TEXT("1.1")));
    TestFalse(TEXT("Innings not complete"), Score->IsInningsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRulesDeliveryTest, "CricketGame.Rules.DeliveryLifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRulesDeliveryTest::RunTest(const FString& Parameters)
{
    UScoreManager* Score = NewObject<UScoreManager>();
    Score->Reset(1);
    UMatchRulesManager* Rules = NewObject<UMatchRulesManager>();
    Rules->Initialize(Score);

    Rules->BeginDelivery();
    Rules->AddCompletedRun();
    Rules->AddCompletedRun();
    Rules->FinalizeDelivery(ECricketDeliveryOutcome::Runs, 0, true);

    TestEqual(TEXT("Two completed runs"), Score->Runs, 2);
    TestEqual(TEXT("One legal ball"), Score->LegalBalls, 1);

    Rules->BeginDelivery();
    Rules->FinalizeDelivery(ECricketDeliveryOutcome::BoundaryFour, 0, true);
    TestEqual(TEXT("Boundary added"), Score->Runs, 6);
    TestEqual(TEXT("Second legal ball"), Score->LegalBalls, 2);
    return true;
}

#endif
