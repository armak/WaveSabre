#include "OrbiterEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

OrbiterEditor::OrbiterEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 460, 100, "Orbiter")
{
}

OrbiterEditor::~OrbiterEditor()
{
}

void OrbiterEditor::Open()
{
	addKnob((VstInt32)Orbiter::ParamIndices::Source, "SIDECHAIN");

	addSpacer();

	addKnob((VstInt32)Orbiter::ParamIndices::Frequency, "FREQ");
	addKnob((VstInt32)Orbiter::ParamIndices::Shape, "SHAPE");
	addKnob((VstInt32)Orbiter::ParamIndices::Rectify, "RECTIFY");
	addKnob((VstInt32)Orbiter::ParamIndices::Bias, "BIAS");

	addSpacer();

	addKnob((VstInt32)Orbiter::ParamIndices::Spread, "SPREAD");
	addKnob((VstInt32)Orbiter::ParamIndices::Amount, "AMOUNT");

	VstEditor::Open();
}
