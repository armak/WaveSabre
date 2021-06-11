#include "ButterEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

ButterEditor::ButterEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 320, 100, "BUTTER")
{
}

ButterEditor::~ButterEditor()
{
}

void ButterEditor::Open()
{
	addKnob((VstInt32)Butter::ParamIndices::Type, "TYPE");
	addSpacer();
	addKnob((VstInt32)Butter::ParamIndices::Order, "ORDER");
	addSpacer();
	addKnob((VstInt32)Butter::ParamIndices::Cutoff, "CUTOFF");
	addSpacer();
	addKnob((VstInt32)Butter::ParamIndices::Q, "Q");

	VstEditor::Open();
}
