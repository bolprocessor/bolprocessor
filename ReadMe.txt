BOL PROCESSOR SOURCE CODE READ ME
=================================

Bol Processor is a program for music composition and improvisation with
real-time MIDI, MIDI file, and Csound output.  It produces music from a set
of rules (a compositional grammar) or from text scores typed or captured
from a MIDI instrument.

Bol Processor is now released as free software (open source) under a
BSD-style license (please see the License.txt file for complete terms).  BP
development is now hosted by Sourceforge at
<http://bolprocessor.sourceforge.net/>.  Developers interested in joining
the project are most welcome to do so.  Please email me for more
information.

At this stage, the source code in CVS is unaltered from the code that I
received from Bernard Bel (except for some file reorganization).  This code
could be considered version 2.9.5 beta (2.9.3 was the last stable public
release; I have a copy of the unreleased 2.9.4 application too).  I did
create a new CodeWarrior Pro 4 project with 68K and PPC MacOS targets.  BP2
will only compile for the Classic MacOS at this time.  One of the goals of
the open-source project will be to port it to other platforms (MacOS X in
particular).

Because the code has not been changed, the BP2 application still thinks that
it is necessary to register.  A registration code is provided in the file
_bp2_key (which will be made invisible when BP2 is run).  Please disregard
all notices from the program or in the documentation about needing to
purchase a registration code.  (Removing the registration system is the
first development priority).

If you want to get started understanding the source code, look in the
"docs-developer" directory; particularly the files "BP2-info.txt" and
"BP2-history.txt".

If you need to create your own project file for a different version of
CodeWarrior or a different IDE, just add all of the files in source/BP2 and
source/WASTE to an appropriate project template.  (The other directories are
not needed).  Then add the following resource files from the "resources"
directory:
	BP2-MacOS.rsrc
	MidiSerialDriver.rsrc
	ppc-oops.rsrc           (only if compiling a PPC version)

The file "BP2-RMaker.R" is a text version of most of the resources but the
application that compiles them came with Symantec C, so we will probably not
be maintaining this file.

You will need to link your project to the standard Mac and ANSI libraries. 
In addition, you will need to download and install the OMS SDK for Midi and
link to the "OMSGluePPC.lib" static library.  (The "OMSLibPPC.slb" shared
library should be in the system Extensions folder or in the same folder as
the BP2 application).  Depending on where you install the OMS SDK, you may
also need to add the "Headers" and "Libraries" folders to your project
Access Paths or include path.
 
Once you have a compiled and linked BP2 application, try running the
"+sc.checkall" script.  (You may need to do a "check script" operation first
to resolve directory differences.  Currently this script fails for me
because -gr.Visser3 is failing in beta mode).  There are also many example
files in the "data" directory.

A Note regarding optimization levels:  Level 4 in CW4 requires a lot of
memory to compile.  But I had to back the optimizations down to Level 2
anyways to avoid an infinite loop when choosing "New Project" that seemed to
be caused by compiling at Level 3.

If you want to follow or contribute to the developement of BP3, then you
will probably want to join the Bol Processor Development mailing list hosted
with the Sourceforge project.  (The BP2 mailing list at
<bp2-list@yahoogroups.com> is for user discussions).

Please also feel free to contact me if you have problems with compiling the
code or other questions.

Anthony Kozar
akozar@users.sourceforge.net

April 28, 2006
