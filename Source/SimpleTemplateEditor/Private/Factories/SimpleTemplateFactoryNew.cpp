// Copyright Playspace S.L. 2017

#include "SimpleTemplateFactoryNew.h"

#include "SimpleTemplate.h"


/* USimpleTemplateFactoryNew structors
 *****************************************************************************/

USimpleTemplateFactoryNew::USimpleTemplateFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USimpleTemplate::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory overrides
 *****************************************************************************/

UObject* USimpleTemplateFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	USimpleTemplate* SimpleTemplate = NewObject<USimpleTemplate>(InParent, InClass, InName, Flags);
	SimpleTemplate->Status = ETemplateStatus::TS_BeingCreated;
	return SimpleTemplate;
}


bool USimpleTemplateFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
