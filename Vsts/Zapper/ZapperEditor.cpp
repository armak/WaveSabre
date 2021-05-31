#include "ZapperEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

ZapperEditor::ZapperEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 320, 100, "Zapper")
{
}

ZapperEditor::~ZapperEditor()
{
}

void ZapperEditor::Open()
{
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Frequency, "FREQUENCY");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Q, "Q");
	addSpacer();
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Strength, "STRENGTH");

	VstEditor::Open();
}
