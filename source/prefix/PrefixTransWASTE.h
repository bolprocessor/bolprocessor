/*	PrefixTransWASTE.h

	BP2 Mac Carbon Transitional Prefix Header 
	Using WASTE
 */
 
// 1 for debugging memory, 0 otherwise
#define BIGTEST 1

// 1 to execute Beta tests for data validity, 0 otherwise
#define COMPILING_BETA 1

// 1 to use WASTE for text editing, 0 to use TextEdit instead
#define WASTE 1
// this is used by WASTE to cut out some non-Carbon-compliant code
#define SystemSevenFiveOrLater 1
// turn off WASTE features that we do not need
#define WASTE_OBJECTS  0
#define WASTE_USE_UPPS 0

// disable built-in MIDI driver at compile time (not finished yet) - 010507 akozar
#define USE_BUILT_IN_MIDI_DRIVER 1

// use pre-Appearance edit text boxes
#define USE_OLD_EDIT_TEXT 1

#define EXPERIMENTAL 1
