/***************************************************************************
 *   Copyright (C) 2008 by Steve Merrony                                   *
 *      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MIDI_DATA_H
#define MIDI_DATA_H

#ifdef Q_WS_X11
#include <alsa/asoundlib.h>
#endif
#ifdef Q_WS_WIN
#include <windows.h>
#include <mmsystem.h>
#endif

#include <QMutex>
#include <QStringList>
#include <QWaitCondition>

const int 	EWI_NUM_PATCHES 		= 100;
const int 	EWI_PATCH_LENGTH 		= 206;
const int 	EWI_PATCHNAME_LENGTH 	= 32;
const int	EWI_SOUNDBANK_MAX_HEADER_LENGTH = 0x450;	// looks safe from observation
// below is the sequence which appears to start the real body of SQS files, there
// seem to be various false BODYs before the main one we care about
const char	EWI_SQS_BODY_START[]	= { 'B', 'O', 'D', 'Y', 0x00, 0x00, 0x50, 0x78, 0xf0, 0x47, 0x64, 0x7f, 0x00, 0x00 };
const int	EWI_SQS_MAX_HEADER_LENGTH = 0x1000;			// looks ok from observation
const char 	EWI_EDIT 				= 0x20;
const char 	EWI_SAVE 				= 0x00;
#ifdef Q_WS_WIN
const int   WIN32_DUMMY_CLIENT		= 0;
//const int	WIN32_NUM_BUFS = 2;
const int	WIN32_BUF_SIZE = 1024;
#endif

/**
	@author Steve Merrony
 */
	typedef struct
{
	int my_client;
	int seq_client;
#ifdef Q_WS_X11
	snd_seq_t *seq_handle;
	snd_seq_event_t ev;
#endif
#ifdef Q_WS_WIN
	HMIDIOUT	outHandle;
	HMIDIIN		inHandle;
#endif
} midi_seq;

	typedef struct
{
	int my_port;
	int seq_port;
} midi_port;

typedef struct {
	char	LSB;
	char	MSB;
	char	offset;
} NRPN;

typedef struct {
	NRPN	nrpn;
	char	octave; 	// int 64 +/-2
	char	semitone;	// int 64 +/-12
	char	fine;		// int -50 - +50 cents
	char	beat;		// int 0% - 100%
	char		filler1;	// this really is unused (for firmware 2.3 anyway)
	char	sawtooth;	// %
	char	triangle;	// %
	char	square;		// %
	char	pulseWidth;	// %
	char	PWMfreq;	// %
	char	PWMdepth;	// %
	char	sweepDepth;	// %
	char	sweepTime;	// %
	char	breathDepth;	// %  ?-50/+50?
	char	breathAttain;	// %
	char	breathCurve;	// %  ? =50+50?
	char	breathThreshold;// %
	char	level;		// %
} osc;

typedef struct {
	NRPN	nrpn;
	char	mode;
	char	freq;
	char	Q;
	char	keyFollow;
	char	breathMod;
	char	LFOfreq;
	char	LFOdepth;
	char	LFObreath;
	char	LFOthreshold;
	char	sweepDepth;
	char	sweepTime;
	char	breathCurve;	
} filter;


union patch_t
{
	char whole_patch[EWI_PATCH_LENGTH];
	struct
	{
		char			header[4];
		char			mode;			// 0x00 to store, 0x20 to edit
		unsigned char	patch_num;
		char    				filler2[6];
		char			name[EWI_PATCHNAME_LENGTH];
		osc				osc1;			// 64,18
		osc				osc2;			// 65,18
		filter			oscFilter1;		// 72,12
		filter			oscFilter2;		// 73,12
		filter			noiseFilter1;	// 74,12
		filter			noiseFilter2;	// 75,12
		NRPN			antiAliasNRPN;	// 79,3
		char			antiAliasSwitch;
		char			antiAliasCutoff;
		char			antiAliasKeyFollow;
		NRPN			noiseNRPN;		// 80,3
		char			noiseTime;
		char			noiseBreath;
		char			noiseLevel;
		NRPN			miscNRPN;		// 81,10
		char			bendRange;
		char			bendStepMode;
		char			biteVibrato;
		char			oscFilterLink;
		char			noiseFilterLink;
		char			formantFilter;
		char			osc2Xfade;
		char			keyTrigger;
		char					filler10;
		char			chorusSwitch;
		NRPN			ampNRPN;		// 88,3
		char			biteTremolo;
		char			ampLevel;
		char			octaveLevel;
		NRPN			chorusNRPN;		// 112,9
		char			chorusDelay1;
		char			chorusModLev1;
		char			chorusWetLev1;
		char			chorusDelay2;
		char			chorusModLev2;
		char			chorusWetLev2;
		char			chorusFeedback;
		char			chorusLFOfreq;
		char			chorusDryLevel;
		NRPN			delayNRPN;		// 113,5
		char			delayTime;
		char			delayFeedback;
		char			delayDamp;
		char			delayLevel;
		char			delayMix;		// ZJ
		NRPN			reverbNRPN;		// 114,5
		char			reverbMix;		// ZJ
		char			reverbLevel;
		char			reverbDensity;
		char			reverbTime;
		char			reverbDamp;
		char			trailer_f7;		// 0xf7 !!!
	} parameters;
};


class midi_data{
	
public:
    midi_data();
    ~midi_data();
	
	void createOurMIDIports();
	void sendPanic();
	bool requestPatch( char );
	void sendLiveControl(int, int, int );
	void sendCC( int, int, int = 0 );
	void sendSysEx(char *, int );
	void sendPatch( patch_t, char = EWI_SAVE );
	
	void scanPorts();
	void connectOutput( int, int );
	void connectInput( int, int );
	void disconnectInput();
	void disconnectOutput();
//#ifdef Q_WS_WIN
//	void CALLBACK win32MIDIinCallback( HMIDIIN, UINT, DWORD, DWORD, DWORD );
//#endif
	QString getPatchName( char * );
	QString hexify( char *bin_patch, bool with_spaces = true );
	patch_t dehexify( QString hex_patch, bool with_spaces );
	
	midi_seq  seq;
	midi_port inp_port, out_port;
	
	bool    verboseMode;
	
	QMutex  mymutex;
	QWaitCondition sysexDone;
	
	int			last_patch_loaded;
	patch_t 	patches[EWI_NUM_PATCHES];
	
	QStringList inPortList, outPortList;
	QList<int>  inPortClients, outPortClients;
	QList<int>  inPortPorts, outPortPorts;
	int         connectedInPort, connectedInClient;
	int         connectedOutPort, connectedOutClient;
#ifdef Q_WS_WIN	
	char		*mmsg; 
	MIDIHDR 	*buf;
	LPMIDIHDR   lpMIDIHeader;
#endif	
};

#endif
