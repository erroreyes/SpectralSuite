#include "SpectralGateParameters.h"

SpectralGateParameters::SpectralGateParameters(AudioProcessor * processor) : PluginParameters(processor) {
	const String dbSuffix = " dB";
	createAndAddParameter(std::make_unique<AudioParameterFloat>(
		ParameterID("cutoff", 1),
		"Cutoff",
		NormalisableRange<float>(0.0f, 1.0f),
		0.6f, "",
		AudioProcessorParameter::Category::genericParameter,
		[dbSuffix](float value, int) {
			return String(Decibels::gainToDecibels(value), 0) + dbSuffix;
		},
		[dbSuffix](const String& text) {
			return  Decibels::decibelsToGain(text.dropLastCharacters(dbSuffix.length()).getFloatValue());
		}
	));

	createAndAddParameter(std::make_unique<AudioParameterFloat>(
        ParameterID("balance", 1),
		"Weak/Strong Balance",
		NormalisableRange<float>(0.0f, 1.0f), 0.7f, "",
		AudioProcessorParameter::Category::genericParameter
	));
}

