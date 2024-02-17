/*
  ==============================================================================

    SettingsPage.h
    Created: 17 Apr 2019 9:48:04pm
    Author:  rem_d

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

//==============================================================================
/*
*/
class SettingsPage : public Component
{
public:
	class Listener {
	public:
		virtual void onPropertiesChanged() = 0;
        virtual ~Listener(){};
	};

    SettingsPage();
    ~SettingsPage();

    void paint (Graphics&) override;
    void resized() override;
	void setListener(Listener* newListener) { this->listener = newListener; }
	void setProperties(Array<PropertyComponent*> newProperties) { 		
		propertyPanel.clear();		
		propertyPanel.addProperties(newProperties);
	}

	//Array<PropertyComponent*> createSliders(int howMany);

private:
	void backButtonClicked();

	PropertyPanel propertyPanel;
	DrawableButton backButton;
	Listener* listener;
    std::unique_ptr<Drawable> settingsIcon;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsPage)
};
