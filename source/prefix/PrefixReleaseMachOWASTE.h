/*	PrefixReleaseMachOWASTE.h

	BP2 MacOS X Release Target Prefix Header
	
	Based on "Carbon Mach-O WASTE" target.
	Using WASTE
 */
 
#include <MSL MacHeadersMach-O.h>

// our macro to indicate a Mach-O compile
#define BP_MACHO 1

// 1 for debugging memory, 0 otherwise
#define BIGTEST 0

// 1 to execute Beta tests for data validity, 0 otherwise
#define COMPILING_BETA 0

// 1 to use WASTE for text editing, 0 to use TextEdit instead
#define WASTE 1
// this is used by WASTE to cut out some non-Carbon-compliant code
#define SystemSevenFiveOrLater 1
// turn off WASTE features that we do not need
#define WASTE_OBJECTS  0
#define WASTE_USE_UPPS 0

// disable built-in MIDI driver and OMS Midi driver
#define USE_BUILT_IN_MIDI_DRIVER 0
#define USE_OMS 0

// enable real-time MIDI driver (CoreMIDI in this case)
#define WITH_REAL_TIME_MIDI 1

#define EXPERIMENTAL 0
