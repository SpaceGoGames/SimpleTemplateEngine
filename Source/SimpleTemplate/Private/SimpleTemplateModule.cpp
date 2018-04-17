// Copyright Playspace S.L. 2017

#include "ISimpleTemplate.h"

/**
 * Implements the SimpleTemplate module.
 */
class FSimpleTemplateModule
	: public ISimpleTemplateModule
{
public:

	//~ IModuleInterface interface

	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};


IMPLEMENT_MODULE(FSimpleTemplateModule, SimpleTemplate);

DEFINE_LOG_CATEGORY(LogSTE);
