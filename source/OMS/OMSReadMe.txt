Note:  The information below only applies to building Bol Processor for Mac OS 9.
On Mac OS X, BP2 uses CoreMIDI exclusively and the OMS SDK is not needed.

-------------

Bol Processor currently requires the headers and libraries for Opcode's OMS
Midi software to compile.  The most recent version of these are found in the
OMS 2.0 SDK dated 28-Jan-98.

These files CANNOT be placed in Sourceforge CVS since their distribution
license does not meet the open source definition.  However, BP2's license does
allow linking with them.  It is hoped that this non-open-source dependency
can be eliminated in the future (perhaps by moving to Midishare).

Opcode, Inc.  no longer distributes the SDK, but it was recently available
at this location:

http://www.rawsound.com/programs/OMS_SDK.sit

or

http://www.rawsound.com/programs/OMS_SDK.hqx


Anthony Kozar
April 7, 2006
