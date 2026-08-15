// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistActivatable.generated.h"

/**
 *  "Can be switched on/off" contract. A switch drives any actor that implements
 *  this without knowing its concrete type, so a switch could open a door today
 *  and disable a camera later with no change to the switch.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UHeistActivatable : public UInterface
{
	GENERATED_BODY()
};

class IHeistActivatable
{
	GENERATED_BODY()

public:

	/** Turn on (e.g. open the door) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Activatable")
	void Activate(AActor* Activator);

	/** Turn off (e.g. close the door) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Activatable")
	void Deactivate(AActor* Activator);
};
