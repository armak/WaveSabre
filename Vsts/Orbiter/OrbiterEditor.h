#ifndef __ORBITEREDITOR_H__
#define __ORBITEREDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class OrbiterEditor : public VstEditor
{
public:
	OrbiterEditor(AudioEffect *audioEffect);
	virtual ~OrbiterEditor();

	virtual void Open();
};

#endif
