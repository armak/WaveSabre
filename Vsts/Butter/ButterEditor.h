#ifndef __BUTTEREDITOR_H__
#define __BUTTEREDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ButterEditor : public VstEditor
{
public:
	ButterEditor(AudioEffect *audioEffect);
	virtual ~ButterEditor();

	virtual void Open();
};

#endif
