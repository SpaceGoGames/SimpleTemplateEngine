// Copyright Playspace S.L. 2017

#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "SimpleTemplateFactory.generated.h"


/**
 * Implements a factory for USimpleTemplate objects.
 */
UCLASS(hidecategories=Object)
class USimpleTemplateFactory
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
