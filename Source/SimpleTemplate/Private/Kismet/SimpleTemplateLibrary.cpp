// Copyright Playspace S.L. 2017

#include "Kismet/SimpleTemplateLibrary.h"

FString USimpleTemplateLibrary::Interpret(USimpleTemplate* SimpleTemplate, TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
{
	return SimpleTemplate->Interpret(DataProvider);
}

USimpleTemplateData* USimpleTemplateLibrary::NewDataProvider(const FString& Data)
{
	USimpleTemplateData* DataProvider = NewObject<USimpleTemplateData>();
	DataProvider->SetData(Data);
	return DataProvider;
}