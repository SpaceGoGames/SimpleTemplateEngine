// Copyright Playspace S.L. 2017

#pragma once

#include "Internationalization/Text.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Compiler/SimpleTemplateCompiler.h"

#include "SimpleTemplate.generated.h"


/**
 * Implements an asset that can be used to store arbitrary text, such as notes
 * or documentation.
 */
UCLASS(BlueprintType, hidecategories=(Object))
class SIMPLETEMPLATE_API USimpleTemplate
	: public UObject
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	/** Holds the stored text. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Simple Template")
	FText Template;

	// TemplateError
	UPROPERTY()
	TArray<FString> LastErrors;

	// A dirty template needs to be compiled
	UPROPERTY()
	uint32 bDirty : 1;

	bool Compile();
#endif

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;

private:
	FTokenArray Tokens;

	/** Add a token to our token list */
	bool AddToken(FToken* token, bool checked = false)
	{
		FString buildError = token->Build();
		if (buildError.IsEmpty())
		{
			Tokens.Add(MakeShareable(token));
			return true;
		}
		LastErrors.Add(buildError);
		return false;
	}
};
