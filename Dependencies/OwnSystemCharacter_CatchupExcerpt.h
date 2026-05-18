// Portfolio excerpt derived from a larger gameplay character class.
// Only the catchup-relevant declaration is kept here.

#pragma once

#include "CoreMinimal.h"

class AOwnSystemCharacter
{
public:
	// Used by the catchup layer to detect whether the NPC is still in its
	// definition/load phase during World Partition stream-in.
	bool IsCharacterPendingLoad() const;

private:
	bool bHasCharacterVisual = false;
	bool bBaseAppearanceLoaded = false;
	bool bHasOutstandingLoadHandles = false;
	bool bCharacterDefinitionLoadHandleActive = false;
};
