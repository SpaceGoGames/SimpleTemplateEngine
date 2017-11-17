// Copyright Playspace S.L. 2017

#pragma once

#include "Compiler/SimpleTemplateCompiler.h"
#include "SimpleTemplate.h"
#include "SimpleTemplateData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectConverter.h"
#include "SimpleTemplateLibrary.generated.h"

UCLASS()
class SIMPLETEMPLATE_API USimpleTemplateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine")
	static FString Interpret(USimpleTemplate* SimpleTemplate, USimpleTemplateData* Data);
};
