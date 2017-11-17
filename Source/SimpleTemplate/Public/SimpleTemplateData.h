// Copyright Playspace S.L. 2017

#pragma once

#include "Internationalization/Text.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "JsonObjectWrapper.h"

#include "SimpleTemplateData.generated.h"

/**
 *Object used to provide a SimpleTemplate with data
 */
UCLASS(BlueprintType, hidecategories=(Object))
class SIMPLETEMPLATE_API USimpleTemplateData
    : public UObject
{
    GENERATED_BODY()

public:

    /** The template data */
    UPROPERTY(EditAnywhere, Category = "Data")
    FJsonObjectWrapper Data;
};
