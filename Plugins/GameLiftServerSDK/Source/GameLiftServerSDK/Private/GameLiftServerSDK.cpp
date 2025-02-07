// AMAZON CONFIDENTIAL

/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "GameLiftServerSDKPrivatePCH.h"
#include "Core.h"
#include "ModuleManager.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "FGameLiftServerSDKModule"

void* FGameLiftServerSDKModule::GameLiftServerSDKLibraryHandle = nullptr;

void FGameLiftServerSDKModule::StartupModule()
{
#if PLATFORM_WINDOWS
    #if PLATFORM_64BITS
        #if WITH_GAMELIFT
            FString BaseDir = IPluginManager::Get().FindPlugin("GameLiftServerSDK")->GetBaseDir();
            const FString SDKDir = FPaths::Combine(*BaseDir, TEXT("ThirdParty"), TEXT("GameLiftServerSDK"));
            const FString LibName = TEXT("aws-cpp-sdk-gamelift-server");
            const FString LibDir = FPaths::Combine(*SDKDir, TEXT("Win64"));
            if (!LoadDependency(LibDir, LibName, GameLiftServerSDKLibraryHandle))
            {
                FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT(LOCTEXT_NAMESPACE, "Failed to load aws-cpp-sdk-gamelift-server library. Plug-in will not be functional."));
                FreeDependency(GameLiftServerSDKLibraryHandle);
            }
        #endif
    #endif
#endif
}

bool FGameLiftServerSDKModule::LoadDependency(const FString& Dir, const FString& Name, void*& Handle)
{
    FString Lib = Name + TEXT(".") + FPlatformProcess::GetModuleExtension();
    FString Path = Dir.IsEmpty() ? *Lib : FPaths::Combine(*Dir, *Lib);

    Handle = FPlatformProcess::GetDllHandle(*Path);

    if (Handle == nullptr)
    {
        return false;
    }

    return true;
}

void FGameLiftServerSDKModule::FreeDependency(void*& Handle)
{
#if !PLATFORM_LINUX
    if (Handle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(Handle);
        Handle = nullptr;
    }
#endif
}

void FGameLiftServerSDKModule::ShutdownModule()
{
    FreeDependency(GameLiftServerSDKLibraryHandle);
}

FGameLiftStringOutcome FGameLiftServerSDKModule::GetSdkVersion() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::GetSdkVersion();
    if (outcome.IsSuccess()){
        return FGameLiftStringOutcome(outcome.GetResult());
    }
    else {
        return FGameLiftStringOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftStringOutcome("");
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::InitSDK() {
#if WITH_GAMELIFT
    auto initSDKOutcome = Aws::GameLift::Server::InitSDK();
    if (initSDKOutcome.IsSuccess()) {
        return FGameLiftGenericOutcome(nullptr);
    }
    else{
        return FGameLiftGenericOutcome(FGameLiftError(initSDKOutcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::ProcessEnding() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::ProcessEnding();
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::ActivateGameSession() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::ActivateGameSession();
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::TerminateGameSession() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::TerminateGameSession();
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::AcceptPlayerSession(const FString& playerSessionId) {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_UTF8(*playerSessionId));
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::RemovePlayerSession(const FString& playerSessionId) {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::RemovePlayerSession(TCHAR_TO_UTF8(*playerSessionId));
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

void OnActivateFunctionInternal(Aws::GameLift::Server::Model::GameSession gameSession, void* state) {
    FProcessParameters* processParameters = (FProcessParameters*)state;
    processParameters->OnActivateFunction(gameSession);
}

void OnTerminateFunctionInternal(void* state) {
    FProcessParameters* processParameters = (FProcessParameters*)state;
    processParameters->OnTerminateFunction();
}

bool OnHealthCheckInternal(void* state) {
    FProcessParameters* processParameters = (FProcessParameters*)state;
    return processParameters->OnHealthCheckFunction();
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::ProcessReady(FProcessParameters &processParameters) {
#if WITH_GAMELIFT
    const char* logPaths[MAX_LOG_PATHS];
    for (int i = 0; i < processParameters.logParameters.Num(); i++)
    {
        logPaths[i] = TCHAR_TO_UTF8(*processParameters.logParameters[i]);
    }

    Aws::GameLift::Server::ProcessParameters processParams = Aws::GameLift::Server::ProcessParameters(
        OnActivateFunctionInternal,
        &(processParameters),
        OnTerminateFunctionInternal,
        &(processParameters),
        OnHealthCheckInternal,
        &(processParameters),
        processParameters.port,
        Aws::GameLift::Server::LogParameters(logPaths, processParameters.logParameters.Num()));

    auto outcome = Aws::GameLift::Server::ProcessReady(processParams);
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftGenericOutcome FGameLiftServerSDKModule::UpdatePlayerSessionCreationPolicy(EPlayerSessionCreationPolicy policy)
{
#if WITH_GAMELIFT
    Aws::GameLift::Server::Model::PlayerSessionCreationPolicy internalPolicy = Aws::GameLift::Server::Model::PlayerSessionCreationPolicyMapper::GetPlayerSessionCreationPolicyForName(TCHAR_TO_UTF8(*GetNameForPlayerSessionCreationPolicy(policy)));
    auto outcome = Aws::GameLift::Server::UpdatePlayerSessionCreationPolicy(internalPolicy);
    if (outcome.IsSuccess()){
        return FGameLiftGenericOutcome(nullptr);
    }
    else {
        return FGameLiftGenericOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftGenericOutcome(nullptr);
#endif
}

FGameLiftStringOutcome FGameLiftServerSDKModule::GetGameSessionId() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::GetGameSessionId();
    if (outcome.IsSuccess()){
        return FGameLiftStringOutcome(outcome.GetResult());
    }
    else {
        return FGameLiftStringOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftStringOutcome("");
#endif
}

FGameLiftLongOutcome FGameLiftServerSDKModule::GetTerminationTime() {
#if WITH_GAMELIFT
    auto outcome = Aws::GameLift::Server::GetTerminationTime();
    if (outcome.IsSuccess()) {
        return FGameLiftLongOutcome(outcome.GetResult());
    }
    else {
        return FGameLiftLongOutcome(FGameLiftError(outcome.GetError()));
    }
#else
    return FGameLiftLongOutcome(-1);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGameLiftServerSDKModule, GameLiftServerSDK)
