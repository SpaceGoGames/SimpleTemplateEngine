// Copyright Playspace S.L. 2017

#pragma once

#include "Internationalization/Text.h"
#include "ISimpleTemplate.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "SimpleTemplateData.h"
#include "Compiler/SimpleTemplateCompiler.h"

#include "Interfaces/SimpleTemplateDataProvider.h"

#include "Serialization/JsonTypes.h"

#include "SimpleTemplate.generated.h"

/**
* Enumerates states a template can be in.
*/
UENUM()
enum class ETemplateStatus : uint8
{
	/** Template is in an unknown state. */
	TS_Unknown,
	/** Template has been modified but not recompiled. */
	TS_Dirty,
	/** Template tried but failed to be compiled. */
	TS_Error,
	/** Template has been compiled since it was last modified. */
	TS_UpToDate,
	/** Template is in the process of being created for the first time. */
	TS_BeingCreated
};

/**
 * Asset used to implement complex template replace logic.
 */
UCLASS(BlueprintType, hidecategories=(Object))
class SIMPLETEMPLATE_API USimpleTemplate
	: public UObject
{
	GENERATED_BODY()

public:

#if WITH_EDITORONLY_DATA
	/** Holds the stored text. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Simple Template")
	FText Template;

	// TemplateError
	UPROPERTY()
	TArray<FString> LastErrors;

	// Line and chars used to goto in case of an error
	UPROPERTY()
	uint32 LineNumber;
	UPROPERTY()
	uint32 CharacterNumber;
#endif

#if WITH_EDITOR
	bool Compile();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	bool IsUpToDate() const
	{
		return ETemplateStatus::TS_UpToDate == Status;
	}

	bool IsPossiblyDirty() const
	{
		return (ETemplateStatus::TS_Dirty == Status) || (ETemplateStatus::TS_Unknown == Status);
	}

	bool IsError() const
	{
		return (ETemplateStatus::TS_Error == Status);
	}

	FString Interpret(TScriptInterface<ISimpleTemplateDataProvider> DataProvider);
	FString Interpret(TSharedPtr<FJsonObject> Data);

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;

public:

	/** The current status of this template */
	UPROPERTY()
	ETemplateStatus Status;

	/** Compiled tokens */
	FTokenArray Tokens;
};
