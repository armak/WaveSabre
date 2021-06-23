#ifndef __MULTIBANDEDITOR_H__
#define __MULTIBANDEDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class MultibandEditor : public VstEditor
{
public:
	MultibandEditor(AudioEffect *audioEffect);
	virtual ~MultibandEditor();

	virtual void Open();
};

#endif
