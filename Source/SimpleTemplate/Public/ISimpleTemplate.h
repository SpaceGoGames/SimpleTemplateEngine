// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
* The public interface to this module.  In most cases, this interface is only public to sibling modules
* within this plugin.
*/
class ISimpleTemplateModule : public IModuleInterface
{

public:

	/**
	* Singleton-like access to this module's interface.  This is just for convenience!
	* Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	*
	* @return Returns singleton instance, loading the module on demand if needed
	*/
	static inline ISimpleTemplateModule& Get()
	{
		return FModuleManager::LoadModuleChecked<ISimpleTemplateModule>("SimpleTemplate");
	}
};

DECLARE_LOG_CATEGORY_EXTERN(LogSTE, Verbose, All);