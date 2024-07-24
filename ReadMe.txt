BOL PROCESSOR SOURCE CODE READ ME
=================================

Bol Processor 3 (BP3) is a program for music composition and improvisation
with real-time MIDI, MIDI file, and Csound output.  It produces music from a
set of rules (a compositional grammar) or from text scores typed or captured
from a MIDI instrument.

BP3 compiles and runs in the MacOS, Linux and Windows environments.

Bol Processor is now released as free software (open source) under a
BSD-style license (please see the License.txt file for complete terms).  
BP development is now hosted by GitHub at

           https://github.com/bolprocessor/bolprocessor

Developers interested in joining the project are most welcome to do so. 
Please email us for more information: contact@bolprocessor.org


One of the goals of the open-source project has been to port BP2/BP3 to other
platforms.

Further development will aim to begin breaking BP3 up into modules to form several
cross-platform libraries corresponding to the major non-GUI components
(grammar productions, sound-object prototypes, the time-setting algorithm
and "MakeSound" which outputs items to various formats, etc.).  These
libraries will provide APIs so that other programs can reuse parts of "BP3",
creating new graphical interfaces that run on other platforms (Linux or
Windows) or embedding BP3 algorithms into custom tools.

If you would like to help with any stage of this process, please get in
contact with us either privately or via the mailing lists:

The BP users help forum: https://sourceforge.net/p/bolprocessor/discussion/555617
BP open discussion forum: https://sourceforge.net/p/bolprocessor/discussion/555616
BP developers list: https://sourceforge.net/projects/bolprocessor/lists/bolprocessor-devel

If you want to get started understanding the source code, look in the
"docs-developer" directory; particularly the files "BP2-info.txt" and
"BP2-history.txt".

If you need to create your own project file for a different version of
Xcode, CodeWarrior, or a different IDE, just add all of the files in
source/BP3 to an appropriate project template. The other directories are
not needed.

Please also feel free to contact us if you have problems with compiling the
code or if you have any other questions.

Thanks for checking out Bol Processor!

Bernard Bel                           Anthony Kozar
