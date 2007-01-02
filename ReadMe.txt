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

The source code for this release has been altered in only minimal ways from
the code that I received from Bernard Bel.  This is version 2.9.5 final of
the project. (2.9.3 was the last stable public release under the shareware
model; There was also an unreleased 2.9.4 application).  I have created a
new CodeWarrior Pro 4 project with 68K and PPC MacOS targets.

BP2 will only compile for the Classic MacOS at this time.  One of the goals
of the open-source project is to port it to other platforms (MacOS X in
particular).  Stage one of this process will begin immediately after this
release of version 2.9.5.  That phase of development will involve porting
BP2 to the Carbon API from Apple so that it can run under MacOS X (and OS
8/9 as well).  The projected completion date for this phase is April 1,
2007.

Stage two will be to begin breaking BP up into modules including several
cross-platform libraries for the major components that can be reused in
other programs.  It will then be possible to create new interfaces for these
parts of BP3 including interfaces that run on other platforms (Linux,
Windows, etc.).

If you would like to help with either stage of this process, please get in
contact with us either privately or via the mailing lists.

Changes since version 2.9.5 beta include the disablement of the shareware
registration system, updates to the BP2 Help file to describe the
open-source licensing model, and minor cosmetic changes to the About box and
File menu. You can view the BP2 license by opening the About box and
clicking the 'License' button.  Additional examples have also been added to
CVS.

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

A note regarding optimization levels:  Level 4 in CW4 requires a lot of
memory to compile.  But I had to back the optimizations down to Level 2
anyways to avoid an infinite loop when choosing "New Project" that seemed to
be caused by compiling at Level 3.

If you want to follow or contribute to the development of BP3, then you
will probably want to join the Bol Processor Development mailing list hosted
with the Sourceforge project.  (The BP2 mailing list at
<bp2-list@yahoogroups.com> is for user discussions).

Please also feel free to contact me if you have problems with compiling the
code or other questions.

Anthony Kozar
anthonykozar@sbcglobal.net

January 2, 2007
