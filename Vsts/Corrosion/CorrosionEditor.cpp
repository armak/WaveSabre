#include "CorrosionEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

CorrosionEditor::CorrosionEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 400, 100, "Corrosion")
{
}

CorrosionEditor::~CorrosionEditor()
{
}

void CorrosionEditor::Open()
{
	addKnob((VstInt32)Corrosion::ParamIndices::InputGain, "GAIN");
	addSpacer();
	addKnob((VstInt32)Corrosion::ParamIndices::Twist, "TWIST");
	addSpacer();
	addKnob((VstInt32)Corrosion::ParamIndices::Saturation, "SATURATION");
	addSpacer();
	addKnob((VstInt32)Corrosion::ParamIndices::Oversampling, "OVERSAMPLING");
	addSpacer();
	addKnob((VstInt32)Corrosion::ParamIndices::DryWet, "DRY/WET");

	VstEditor::Open();
}
