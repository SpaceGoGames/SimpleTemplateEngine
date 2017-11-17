// Copyright Playspace S.L. 2017

#include "Kismet/SimpleTemplateLibrary.h"

FString USimpleTemplateLibrary::Interpret(USimpleTemplate* SimpleTemplate, USimpleTemplateData* Data)
{
	return SimpleTemplate->Interpret(Data);
}