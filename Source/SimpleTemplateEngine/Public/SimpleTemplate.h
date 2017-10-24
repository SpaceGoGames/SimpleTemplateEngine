// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Internationalization/Text.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Compiler/SimpleTemplateCompiler.h"

#include "SimpleTemplate.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSTE, Verbose, All);


/**
 * Implements a compile template
 */
UCLASS(BlueprintType, hidecategories=(Object))
class SIMPLETEMPLATEENGINE_API USimpleTemplate
	: public UObject
{
	GENERATED_BODY()

public:

	/** The template text */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Simple Template")
	FText Template;

#if WITH_EDITORONLY_DATA
	/** All erros or template yields */
	UPROPERTY()
	TArray<FString> TemplateErrors;
#endif

	// UObject interface
	virtual void Serialize(FArchive& Ar) override;

	/** Compiling a template into a reusable format at runtime */
	bool Stuff();

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
		else
		{
			UE_LOG(LogSTE, Error, TEXT("Failed to add token '%s'"), *buildError);
			TemplateErrors.Add(buildError);
		}
		check(!checked || buildError.IsEmpty());
		return false;
	}
};
