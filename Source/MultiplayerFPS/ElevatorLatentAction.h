#pragma once

#include "LatentActions.h"
#include "Math/Transform.h"
#include "Kismet/KismetMathLibrary.h"
#include "Elevator.h"

class ElevatorLatentAction : public FPendingLatentAction {
private:
    AElevator* ElevatorActor;
    FTransform InitialTransform;
    FTransform TargetTransform;
    FTransform CurrentTransform;
    float InterpolationTime;
    float& DeltaTime;
    float ElapsedTime;

    FName ExecutionFunction;
    int32 OutputLink;
    FWeakObjectPtr CallbackTarget;
public:
    ElevatorLatentAction(AElevator* ElevatorActor, FTransform InitialTransform, FTransform TargetTransform, float InterpolationTime, float& DeltaTime, const FLatentActionInfo& LatentInfo) : 
            ElevatorActor(ElevatorActor),
            InitialTransform(InitialTransform),
            TargetTransform(TargetTransform),
            CurrentTransform(InitialTransform),
            InterpolationTime(InterpolationTime),
            DeltaTime(DeltaTime),
            ExecutionFunction(LatentInfo.ExecutionFunction),
            OutputLink(LatentInfo.Linkage),
            CallbackTarget(LatentInfo.CallbackTarget) {
        ElapsedTime = 0.0f;
    }

    virtual void UpdateOperation(FLatentResponse& Response) override {
	    ElapsedTime += DeltaTime;
        CurrentTransform = UKismetMathLibrary::TLerp(CurrentTransform, TargetTransform, ElapsedTime / InterpolationTime, ELerpInterpolationMode::DualQuatInterp);
        ElevatorActor->SetActorTransform(CurrentTransform);
        
        Response.FinishAndTriggerIf(CurrentTransform.EqualsNoScale(TargetTransform, 0.01f), ExecutionFunction, OutputLink, CallbackTarget);
    }
};
