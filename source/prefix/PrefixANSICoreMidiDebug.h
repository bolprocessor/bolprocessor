/*	PrefixANSICoreMidiDebug.h

	BP3 Console App with CoreMIDI Prefix Header
 */

// CoreServices includes the Carbon MacTypes.h header.
// Including it here and #ifdef-ing dupicate definitions in
// MissingMacTypes.h avoids conflicts in CoreMIDIdriver.c.
#include <CoreServices/CoreServices.h>

// our replacements for various Mac types
#include "MissingMacTypes.h"
#include "MissingMacPrototypes.h"
#include "CTextHandles.h"

// don't compile any GUI-specific code
#define BP_CARBON_GUI 0

// our macro to indicate a Mach-O compile
#define BP_MACHO 1

// 1 for debugging memory, 0 otherwise
#define BIGTEST 1

// 1 to execute Beta tests for data validity, 0 otherwise
#define COMPILING_BETA 1

// 1 to use WASTE for text editing, 0 to use TextEdit instead
#define WASTE 0
// this is used by WASTE to cut out some non-Carbon-compliant code
#define SystemSevenFiveOrLater 1
// turn off WASTE features that we do not need
#define WASTE_OBJECTS  0
#define WASTE_USE_UPPS 0

// disable built-in MIDI driver and OMS Midi driver
#define USE_BUILT_IN_MIDI_DRIVER 0
#define USE_OMS 0

// enable CoreMIDI driver
#define WITH_REAL_TIME_MIDI 1
#define USE_COREMIDI 1

#define EXPERIMENTAL 0

#define TRACE_EVENTS 0
