#include "MultibandEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

MultibandEditor::MultibandEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 530, 400, "Multiband")
{
}

MultibandEditor::~MultibandEditor()
{
}

void MultibandEditor::Open()
{
	addKnob((VstInt32)Multiband::ParamIndices::InputGain, "INPUT GAIN");
	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::LowBandCutoff, "LOW FREQ");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::MidBandCutoff, "MID FREQ");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::HighBandCutoff, "HIGH FREQ");
	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Sidechain, "SIDECHAIN");

	startNextRow();

	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band1Threshold, "THRESHOLD");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band2Threshold, "THRESHOLD");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band3Threshold, "THRESHOLD");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band4Threshold, "THRESHOLD");

	startNextRow();

	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band1Ratio, "RATIO");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band2Ratio, "RATIO");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band3Ratio, "RATIO");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band4Ratio, "RATIO");

	startNextRow();

	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band1Attack, "ATTACK");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band2Attack, "ATTACK");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band3Attack, "ATTACK");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band4Attack, "ATTACK");

	startNextRow();

	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band1Release, "RELEASE");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band2Release, "RELEASE");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band3Release, "RELEASE");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band4Release, "RELEASE");

	startNextRow();

	addSpacer();
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band1Gain, "GAIN");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band2Gain, "GAIN");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band3Gain, "GAIN");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Multiband::ParamIndices::Band4Gain, "GAIN");

	VstEditor::Open();
}
