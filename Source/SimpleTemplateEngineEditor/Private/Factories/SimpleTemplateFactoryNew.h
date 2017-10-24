// Copyright Playspace S.L. 2017

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "SimpleTemplate.h"

#include "SimpleTemplateFactoryNew.generated.h"


/**
 * Implements a factory for USimpleTemplate objects.
 */
UCLASS(hidecategories=Object)
class USimpleTemplateFactoryNew
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
