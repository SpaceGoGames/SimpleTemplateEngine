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

	/** Create a simple data provider */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine")
	static USimpleTemplateData* NewDataProvider(const FString& Data);

	/** Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine", meta = (DisplayName = "Interpret (From Provider"))
	static FString Interpret_FromProvider(USimpleTemplate* SimpleTemplate, TScriptInterface<ISimpleTemplateDataProvider> DataProvider);

	/** Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine", meta = (DisplayName = "Interpret (From JSON String"))
	static FString Interpret_FromJSONString(USimpleTemplate* SimpleTemplate, const FString& Data);

	/** Compile & Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine", meta = (DisplayName = "Compile & Interpret (From Provider"))
	static FString CompileAndInterpret_FromProvider(const FString& Template, TScriptInterface<ISimpleTemplateDataProvider> DataProvider);

	/** Compile & Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine", meta = (DisplayName = "Compile & Interpret (From JSON String"))
	static FString CompileAndInterpret_FromString(const FString& Template, const FString& Data);

	/** Compile & Interpret a template */
	UFUNCTION(BlueprintCallable, Category = "Simple Template Engine", meta = (DisplayName = "Compile"))
	static USimpleTemplate* Compile(const FString& Template);

	
};
