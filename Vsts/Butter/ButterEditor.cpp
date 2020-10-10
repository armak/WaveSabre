#include "ButterEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

ButterEditor::ButterEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 250, 100, "BUTTER")
{
}

ButterEditor::~ButterEditor()
{
}

void ButterEditor::Open()
{
	addKnob((VstInt32)Butter::ParamIndices::Cutoff, "CUTOFF");
	addSpacer();
	addKnob((VstInt32)Butter::ParamIndices::Q, "Q");

	VstEditor::Open();
}
