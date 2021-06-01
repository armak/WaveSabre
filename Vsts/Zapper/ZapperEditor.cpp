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
	addKnob((VstInt32)Zapper::ParamIndices::Frequency, "FREQUENCY");
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Q, "Q");
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Strength, "STRENGTH");
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Drywet, "DRY/WET");

	VstEditor::Open();
}
