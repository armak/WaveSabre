#include "CorrosionEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

CorrosionEditor::CorrosionEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 780, 100, "Corrosion")
{
}

CorrosionEditor::~CorrosionEditor()
{
}

void CorrosionEditor::Open()
{
	addKnob((VstInt32)Corrosion::ParamIndices::InputGain, "INPUT");

	addSpacer();

	addKnob((VstInt32)Corrosion::ParamIndices::Rectify, "RECTIFY");
	addKnob((VstInt32)Corrosion::ParamIndices::Twist, "TWIST");
	addKnob((VstInt32)Corrosion::ParamIndices::Fold, "FOLD");
	addKnob((VstInt32)Corrosion::ParamIndices::Saturation, "SATURATE");

	addSpacer();

	addKnob((VstInt32)Corrosion::ParamIndices::ClipDrive, "DRIVE");
	addKnob((VstInt32)Corrosion::ParamIndices::ClipThreshold, "THRESHOLD");
	addKnob((VstInt32)Corrosion::ParamIndices::ClipShape, "SHAPE");

	addSpacer();

	addKnob((VstInt32)Corrosion::ParamIndices::OutputGain, "OUTPUT");
	addKnob((VstInt32)Corrosion::ParamIndices::DryWet, "DRY/WET");

	addSpacer();

	addKnob((VstInt32)Corrosion::ParamIndices::Oversampling, "QUALITY");
	addKnob((VstInt32)Corrosion::ParamIndices::DCBlocking, "DC BLOCK");

	VstEditor::Open();
}
