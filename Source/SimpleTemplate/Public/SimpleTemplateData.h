// Copyright Playspace S.L. 2017

#pragma once

#include "Internationalization/Text.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Dom/JsonObject.h"
#include "Interfaces/SimpleTemplateDataProvider.h"

#include "SimpleTemplateData.generated.h"

/**
 *Object used to provide a SimpleTemplate with data
 */
UCLASS(BlueprintType, hidecategories=(Object))
class SIMPLETEMPLATE_API USimpleTemplateData : public UObject, public ISimpleTemplateDataProvider
{
    GENERATED_BODY()

public:
	USimpleTemplateData();

    /** The template data */
    UPROPERTY(EditAnywhere, Category = "Data")
    FString Json;

	UFUNCTION(BlueprintCallable, Category = "Data")
	bool SetData(const FString& Data);

	virtual TSharedPtr<FJsonObject> GetData() const override;

private:
	FString JsonString;
	TSharedPtr<FJsonObject> JsonPtr;
};
