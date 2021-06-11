#include "MultibandEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

MultibandEditor::MultibandEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 330, 160, "Multiband")
{
}

MultibandEditor::~MultibandEditor()
{
}

void MultibandEditor::Open()
{
	VstEditor::Open();
}
