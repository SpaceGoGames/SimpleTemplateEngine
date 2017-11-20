// Copyright Playspace S.L. 2017

#include "SimpleTemplateData.h"
#include "SimpleTemplate.h"

USimpleTemplateData::USimpleTemplateData()
    : Super()
{
    JsonPtr = MakeShareable(new FJsonObject());
	SetData(Json);
}

bool USimpleTemplateData::SetData(const FString& Data)
{
	if (JsonString.Equals(Data))
	{
		return true;
	}
	JsonString = JsonString;
	auto Reader = TJsonReaderFactory<>::Create(*Data);
	if (FJsonSerializer::Deserialize(Reader, JsonPtr) && JsonPtr.IsValid())
	{
		return true;
	}
	if (JsonPtr.IsValid())
	{
		JsonPtr.Reset();
	}
	JsonPtr = MakeShareable(new FJsonObject());
	UE_LOG(LogSTE, Error, TEXT("Failed to decode Object from JSON string for: %s"), *JsonString);
	return false;
}

TSharedPtr<FJsonObject> USimpleTemplateData::GetData() const
{
    return JsonPtr;
}
