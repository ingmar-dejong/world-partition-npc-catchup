// Portfolio excerpt derived from a larger gameplay character class.
// Only the catchup-relevant implementation is kept here.

#include "OwnSystemCharacter_CatchupExcerpt.h"

bool AOwnSystemCharacter::IsCharacterPendingLoad() const
{
	// No visual yet means appearance data is not ready.
	if (!bHasCharacterVisual)
	{
		return true;
	}

	// Appearance assets are still loading.
	if (!bBaseAppearanceLoaded || bHasOutstandingLoadHandles)
	{
		return true;
	}

	// Definition load pipeline is still active.
	return bCharacterDefinitionLoadHandleActive;
}
