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
	addKnob((VstInt32)Zapper::ParamIndices::Frequency, "Frequency");
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Q, "Q");
	addSpacer();
	addKnob((VstInt32)Zapper::ParamIndices::Intensity, "INTENSITY");

	VstEditor::Open();
}
