// Copyright Playspace S.L. 2017
#include "Kismet/SimpleTemplateLibrary.h"

#include "Serialization/JsonTypes.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

USimpleTemplateData* USimpleTemplateLibrary::NewDataProvider(const FString& Data)
{
	USimpleTemplateData* DataProvider = NewObject<USimpleTemplateData>();
	DataProvider->SetData(Data);
	return DataProvider;
}

FString USimpleTemplateLibrary::Interpret_FromProvider(USimpleTemplate* SimpleTemplate, TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
{
	return SimpleTemplate->Interpret(DataProvider);
}

FString USimpleTemplateLibrary::Interpret_FromJSONString(USimpleTemplate* SimpleTemplate, const FString& Data)
{
	const TSharedRef< TJsonReader<> >& Reader = TJsonReaderFactory<>::Create(Data);
	TSharedPtr<FJsonObject> JsonPtr;
	if (FJsonSerializer::Deserialize(Reader, JsonPtr) && JsonPtr.IsValid())
	{
		return SimpleTemplate->Interpret(JsonPtr);
	}
	return FString();
}

FString USimpleTemplateLibrary::CompileAndInterpret_FromProvider(const FString& Template, TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
{
	auto compiler = TTemplateCompilerFactory<TCHAR>::Create(Template);
	if (compiler->Compile())
	{
		FTokenArray Tokens = compiler->GetTokenTree();
		auto interpreter = TTemplateInterpreter::Create(Tokens);
		FString OutString;
		if (interpreter->Interpret(OutString, DataProvider))
		{
			return OutString;
		}
	}
	return FString();
}

FString USimpleTemplateLibrary::CompileAndInterpret_FromString(const FString& Template, const FString& Data)
{
	const TSharedRef< TJsonReader<> >& Reader = TJsonReaderFactory<>::Create(Data);
	TSharedPtr<FJsonObject> JsonPtr;
	if (FJsonSerializer::Deserialize(Reader, JsonPtr) && JsonPtr.IsValid())
	{
		auto compiler = TTemplateCompilerFactory<TCHAR>::Create(Template);
		if (compiler->Compile())
		{
			FTokenArray Tokens = compiler->GetTokenTree();
			auto interpreter = TTemplateInterpreter::Create(Tokens);
			FString OutString;
			if (interpreter->Interpret(OutString, JsonPtr))
			{
				return OutString;
			}
		}
		
	}
	return FString();
}

USimpleTemplate* USimpleTemplateLibrary::Compile(const FString& Template)
{
	auto compiler = TTemplateCompilerFactory<TCHAR>::Create(Template);
	if (compiler->Compile())
	{
		auto SimpleTemplate = NewObject<USimpleTemplate>();
		SimpleTemplate->Tokens = compiler->GetTokenTree();
		return SimpleTemplate;
	}
	return nullptr;
}