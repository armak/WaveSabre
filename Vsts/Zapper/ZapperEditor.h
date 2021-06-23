#ifndef __ZAPPEREDITOR_H__
#define __ZAPPEREDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class ZapperEditor : public VstEditor
{
public:
	ZapperEditor(AudioEffect *audioEffect);
	virtual ~ZapperEditor();

	virtual void Open();
};

#endif
