// Copyright Playspace S.L. 2017

#include "Containers/Array.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "AssetTools/SimpleTemplateActions.h"
#include "Styles/SimpleTemplateEngineEditorStyle.h"

#define LOCTEXT_NAMESPACE "FSimpleTemplateEngineEditor"

class FSimpleTemplateEngineEditor
	: public IModuleInterface
	, public IHasMenuExtensibility
	, public IHasToolBarExtensibility
{
public:

	/** IModuleInterface interface */
	virtual void StartupModule() override
	{
		Style = MakeShareable(new FSimpleTemplateEngineEditorStyle());

		RegisterAssetTools();
		RegisterMenuExtensions();
	}

	virtual void ShutdownModule() override
	{
		UnregisterAssetTools();
		UnregisterMenuExtensions();
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	/** IHasMenuExtensibility interface */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override
	{
		return MenuExtensibilityManager;
	}
	/** IHasMenuExtensibility interface */

	/** IHasToolBarExtensibility interface */
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override
	{
		return ToolBarExtensibilityManager;
	}
	/** IHasToolBarExtensibility interface */

protected:

	/** Asset Tools */
	void RegisterAssetTools()
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		RegisterAssetTypeAction(AssetTools, MakeShareable(new FSimpleTemplateActions(Style.ToSharedRef())));
	}

	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

	/** Registers main menu and tool bar menu extensions. */
	void RegisterMenuExtensions()
	{
		MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
		ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);
	}

	/** Unregisters main menu and tool bar menu extensions. */
	void UnregisterMenuExtensions()
	{
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();
	}

private:
	/** Holds the menu extensibility manager. */
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;

	/** The collection of registered asset type actions. */
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Holds the plug-ins style set. */
	TSharedPtr<ISlateStyle> Style;

	/** Holds the tool bar extensibility manager. */
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;
};

IMPLEMENT_MODULE(FSimpleTemplateEngineEditor, SimpleTemplateEngineEditor)

#undef LOCTEXT_NAMESPACE
