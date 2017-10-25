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

//	virtual UObject* FactoryCreateBinary(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn) override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
