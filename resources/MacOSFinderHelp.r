#include	<Types.r>
#include	<BalloonTypes.r>

/* Finder Balloon help for Bol Processor 2 */

resource 'hfdr' (-5696, purgeable) {
    HelpMgrVersion, hmDefaultOptions, 0, 0, 					/* header information */
    {HMSTRResItem {1000}}
};

resource 'STR ' (1000, purgeable) {							/* help message for app icon */
    "Bol Processor 2 is a program for music composition and "
    "improvisation with real-time MIDI, MIDI file, and Csound output.\n\n"
    "BP2 is free software.  See \"About BP2...\" for more information."
};


/* 'kind' resource for custon Finder kind strings */

resource 'kind' (1000)
{
	'Bel0',
	verUS,
	{
		'TEXT', "BP2 text file",
		'BP02', "BP2 keyboard",
		'BP03', "BP2 sound-object prototypes",
		'BP04', "BP2 decisions",
		'BP05', "BP2 grammar",
		'BP06', "BP2 alphabet",
		'BP07', "BP2 data",
		'BP08', "BP2 interaction",
		'BP09', "BP2 settings",
		'BP10', "BP2 weights",
		'BP11', "BP2 script",
		'BP12', "BP2 glossary",
		'BP13', "BP2 time-base",
		'BP14', "BP2 Csound instruments",
		'BP15', "BP2 MIDI orchestra",
		'Midi', "BP2 MIDI file",
		'AIFF', "AIFF sound file",
		'AIFC', "AIFF-C sound file"
	}
};
