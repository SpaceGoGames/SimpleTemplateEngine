// Copyright Playspace S.L. 2017

#pragma once

#include "Compiler/SimpleTemplateCompiler.h"
#include "SimpleTemplate.h"
#include "SimpleTemplateData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleTemplateLibrary.generated.h"

UCLASS()
class SIMPLETEMPLATE_API USimpleTemplateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine")
	static FString Interpret(USimpleTemplate* SimpleTemplate, TScriptInterface<ISimpleTemplateDataProvider> DataProvider);

	/** Create a simple data provider */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine")
	static USimpleTemplateData* NewDataProvider(const FString& Data);
};