#include "ZapperEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

ZapperEditor::ZapperEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 460, 100, "Zapper")
{
}

ZapperEditor::~ZapperEditor()
{
}

void ZapperEditor::Open()
{
	addKnob((VstInt32)Zapper::ParamIndices::Source, "SIDECHAIN");

	addSpacer();

	addKnob((VstInt32)Zapper::ParamIndices::Frequency, "FREQ");
	addKnob((VstInt32)Zapper::ParamIndices::Shape, "SHAPE");
	addKnob((VstInt32)Zapper::ParamIndices::Rectify, "RECTIFY");
	addKnob((VstInt32)Zapper::ParamIndices::Bias, "BIAS");

	addSpacer();

	addKnob((VstInt32)Zapper::ParamIndices::Spread, "SPREAD");
	addKnob((VstInt32)Zapper::ParamIndices::Amount, "AMOUNT");

	VstEditor::Open();
}
