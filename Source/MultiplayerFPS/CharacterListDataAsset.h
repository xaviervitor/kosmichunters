#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CharacterListDataAsset.generated.h"

USTRUCT(BlueprintType) 
struct FCharacterInfo {
    GENERATED_USTRUCT_BODY()

	FCharacterInfo() {}
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CharacterName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) class USkeletalMesh* ThirdPersonMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) class USkeletalMesh* FirstPersonBody;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) class USkeletalMesh* FirstPersonArms;
};

UCLASS()
class MULTIPLAYERFPS_API UCharacterListDataAsset : public UDataAsset {
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere) TArray<FCharacterInfo> CharacterList;

public:
	FORCEINLINE TArray<FCharacterInfo> GetCharacterList() { return CharacterList; }
};
