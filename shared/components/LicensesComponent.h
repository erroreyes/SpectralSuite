#pragma once

#include "JuceHeader.h"

//==============================================================================
/*
*/
class LicensesComponent : public Component
{
public:
    LicensesComponent();
    ~LicensesComponent();

    void paint (Graphics&) override;
    void resized() override;

private:

    DrawableButton backButton;
    Label licenses;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicensesComponent)
};