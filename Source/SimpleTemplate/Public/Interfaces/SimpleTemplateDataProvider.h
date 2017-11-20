// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "Dom/JsonObject.h"
#include "SimpleTemplateDataProvider.generated.h"

UINTERFACE(BlueprintType)
class SIMPLETEMPLATE_API USimpleTemplateDataProvider : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

/**
* Derive from this class
*/
class SIMPLETEMPLATE_API ISimpleTemplateDataProvider
{
    GENERATED_IINTERFACE_BODY()

    virtual TSharedPtr<FJsonObject> GetData() const = 0;
};
