// Copyright Playspace S.L. 2017

#include "SimpleTemplateFactoryNew.h"
#include "SimpleTemplate.h"

USimpleTemplateFactoryNew::USimpleTemplateFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = USimpleTemplate::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* USimpleTemplateFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<USimpleTemplate>(InParent, InClass, InName, Flags);
}


bool USimpleTemplateFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
