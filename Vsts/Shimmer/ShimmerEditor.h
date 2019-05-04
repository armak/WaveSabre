#ifndef __SHIMMEREDITOR_H__
#define __SHIMMEREDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ShimmerEditor : public VstEditor
{
public:
	ShimmerEditor(AudioEffect *audioEffect);
	virtual ~ShimmerEditor();

	virtual void Open();
};

#endif
