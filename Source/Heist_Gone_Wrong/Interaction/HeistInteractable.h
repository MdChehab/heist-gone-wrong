// Heist Gone Wrong

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeistInteractable.generated.h"

/**
 *  "Can be interacted with" contract. Implemented by throwables, the switch,
 *  the door and the artifact so the interaction component never needs to know
 *  the concrete class it is looking at.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UHeistInteractable : public UInterface
{
	GENERATED_BODY()
};

class IHeistInteractable
{
	GENERATED_BODY()

public:

	/** Short verb for the HUD prompt, e.g. "Pick Up" or "Flip Switch" */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetInteractionPrompt() const;

	/** Whether Interactor may interact with this actor right now */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	bool CanInteract(AActor* Interactor) const;

	/** Run the interaction. Only called when CanInteract returned true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AActor* Interactor);
};
