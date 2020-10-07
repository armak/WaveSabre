#ifndef __CORROSIONEDITOR_H__
#define __CORROSIONEDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class CorrosionEditor : public VstEditor
{
public:
	CorrosionEditor(AudioEffect *audioEffect);
	virtual ~CorrosionEditor();

	virtual void Open();
};

#endif
