/*	PrefixReleaseMachOWASTE.h

	BP2 MacOS X Release Target Prefix Header
	
	Based on "Carbon Mach-O WASTE" target.
	Using WASTE

	20081016: Changed to use system C library in preparation
	for importing to XCode.
 */
 

// Replaced including MSL headers with direct inclusion of Carbon
#ifndef __NOEXTENSIONS__
  #define __NOEXTENSIONS__
#endif
#ifndef __CF_USE_FRAMEWORK_INCLUDES__
  #define __CF_USE_FRAMEWORK_INCLUDES__
#endif
#include <Carbon/Carbon.h>

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
