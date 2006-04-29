* Resources for Bol Processor BP2.9.4 -- February 1999
* These are compiled by Symantec's "RMaker"

/QUIT
-BP2.π.rsrc;; name of output file
rsrcRSED;; ResEdit document

INCLUDE -BP2.misc.rsrc ;; graphic and color resources
INCLUDE -BP2.driver.rsrc ;; created by MIDIdriver.proj

* MENU
* 128,129,130,131,132,133,134,135,136,137

* WIND
* 511,512,513,514,515,516,517,518,519,520,521,522,523,524

* DLOG
* 128,129,130
* 525,526,527,528,529,530,531,532,533,534,535,536,537,538
* 539,540,541,542,543,544,545,546,547,548,549
* 550,551,552,553,554,555,556,557,558,559,560,561,562,563,564,565,566,567

* DITL
* 18,22,128,129,130,131,318,464
* 600,601,602,603,604,605,606,607,608,609,611,612,613,614,615,616,
*	617,618,619,620,621,622,623,624,625,626,627,628,629,630,631,632,633,634,
*	635,636,637,638,639,640,641,642,643,644,645,646,647,648,649,650,651,652,653,654
* 655
* 764,858,1016,1052,1134

* ALRT
* 142,143,144,145,146,147,148,149,150,151,152,153

* STR#
* 300,301,302,303,304,305,306,307

* My convention for refCon in DIALOGS:
* - refCon is (long) type
* - "r" is the rectangle in which buttons should be drawn
* - From LSB to MSB:
* - 2 bits = button type (0=no button, 1=radio, 2=push, 3=check)
* - 6 bits = 4 * (stringlistID - 300)
* - 9 bits = 256 * r.right.  (If 0 then r.right = window width)
* - 9 bits = 131072 * r.bottom. (If 0 then r.bottom = window height)
* - 3 bits = 67108864 * ((r.left-4) / 64)
* - 3 bits = 536870912 * ((r.top-4) / 64)


* -------------- DATA,BUNDLE,FREF --------------- *

TYPE BNDL
MainBundle,152
Bel0 0
ICN#
0 128  1 129  2 130  3 131  4 132  5 139  6 133  7 135  8 134  9 136  10 137  11 138 12 140 13 142 14 144
FREF
0 134  1 135  2 136  3 137  4 138  5 139  6 140  7 141  8 142  9 143  10 144  11 145 12 146 13 147 14 148
 
TYPE FREF
BP2,134
APPL 0	;; Application

TYPE FREF
Keyboard,143
BP02 9	;; -kb

TYPE FREF
SoundObjects,137
BP03 3	;; -mi

TYPE FREF
Decisions,141
BP04 7	;; decisions

TYPE FREF
Grammar,135
BP05 1	;; -gr

TYPE FREF
Alphabet,136
BP06 2	;; -ho

TYPE FREF
Data,138
BP07 4	;; -da

TYPE FREF
Interactive,139
BP08 5	;; -in

TYPE FREF
Settings,140
BP09 6	;; -se

TYPE FREF
Weights,142
BP10 8	;; -wg

TYPE FREF
Script,144
BP11 10	;; +sc

TYPE FREF
Glossary,145
BP12 11	;; -gl

TYPE FREF
TimeBase,146
BP13 12	;; -tb

TYPE FREF
CsoundInstrSpecs,147
BP14 13	;; -cs

TYPE FREF
MIDIorchestra,148
BP15 14	;; -or

TYPE Bel0 = GNRL
 ,0

* -----------------  MENUS -------------------- *

TYPE MENU
Apple,128
\14
About BP2…
(-

TYPE MENU
File,129
File
New Project/N
Load Project/L
(-
Make grammar from table...
Receive MIDI data to file
Send MIDI data from file
Load time pattern
Get info
(-
Open file/O
Clear window;;  Clear window/\1A
Close window/W 
Save file/S
Save file as…
Revert to saved
(-
Load settings
Save settings
Modify ‘-se.startup’
(-
Page Setup…
Print
(-
Quit/Q

TYPE MENU
Edit,130
Edit
Undo/Z
(-
Cut/X
Copy/C
Paste/V
Clear
(-
Select all/A
(-
Pick tool or performance control
Pick grammar procedure


TYPE MENU
Search,131
Search
Find… replace/F
(Enter and find
(Find again
(-
Check variables
List terminal symbols
List reserved words
Balance selection/B

TYPE MENU
Action,132
Action
Compile project/K
Produce items/R
Check determinism
Produce templates
Analyze selection
Play selection/P
Expand selection
Capture selection as…
Show periods
Execute script
Transliterate text file…
(-
Adjust metronom/M
(-
Pause/.
Resume/R
Stop
(-
Reset session time
Tell session time
(-
Type from MIDI [toggle]/J
(-
(Help


TYPE MENU
Layout,133
Layout
9 pt
10 pt
12 pt
14 pt
(-
Change colors
Color graphics
Color text
(-
AZERTY
QWERTY
(-
Use '•'
(-
Piano roll
(-
Split terminal symbols
Split |variables|


TYPE MENU
Misc,134
Misc
Random sequence
Time accuracy
Buffer size
Graphic settings
Default performance values
Default strike mode
File save preferences
Tuning
(-
French convention;;
English convention;;
Indian convention;;
Key numbers;;
(-
Type text
(Use tokens
Smart cursor [toggle]


TYPE MENU
Devices,135
Devices
MIDI
MIDI files
Csound scores
Opcode OMS
(-
MIDI check input
MIDI check output
MIDI orchestra
(-
Csound instruments
(-
OMS input-output
OMS MIDI setup…
OMS studio setup
(-
Modem port
Printer port
(-
MIDI filter


TYPE MENU
Windows,136
Windows
Grammar/G
Alphabet/H
Data/D
(Settings
Metronom/M
Time-base/T
Interaction/I
Glossary
Start string
Edit prototypes/E
Trace/Y
Graphics/U
Control panel/=
Script/-
Scrap!\13 ;; black diamond mark
Keyboard
Notice
(-
Frequently asked questions…
(-
Show latest messages


TYPE MENU
Script,137
Script
Check script syntax
(-

;; Preferences…!\86/\1B = mark for submenu $86...


* --------------- SUBMENUS -------------------- */

*TYPE MENU
*Preferences,136 ;; Preferences… submenu
*     \00
*MIDI
*Startup

* -----------------  WINDOWS -------------------- *

TYPE WIND
Grammar,511
New Grammar
60 100 315 495
Invisible GoAway
0 ;; documentProc
0

TYPE WIND
Alphabet,512
New Alphabet
55 15 305 250
Invisible GoAway
0 ;; documentProc
0

TYPE WIND
StartString,513
Start string(s)
40 5 100 145
Invisible GoAway
0 ;; documentProc
0

TYPE WIND
Message,514
Message
468 0 480 680
Invisible NoGoAway
2
0

TYPE WIND
Graphics,515
Graphics
150 5 320 500
Invisible GoAway
0 ;; documentProc
0

TYPE WIND
Trace,516
Trace
55 20 305 250
Invisible GoAway
0 ;; documentProc
0

TYPE WIND
Info,517
Info
456 426 468 680
Invisible NoGoAway
2
0

TYPE WIND
Data,518
New Data
55 20 310 378
Invisible GoAway
0
0

Type WIND
ScriptWindow,519
Script
116 324 340 510
Invisible GoAway
0
0

TYPE WIND
Scrap,520
Scrap
103 340 480 640
Invisible GoAway
0
0

TYPE WIND
Help,521
Help
360 0 480 640
Invisible GoAway
0
0

TYPE WIND
Notice,522
Notice
55 20 310 378
Invisible GoAway
0
0


TYPE WIND
Glossary,523
Glossary
80 12 325 500
Invisible GoAway
0
0


TYPE WIND
Interaction,524
Interaction
80 12 325 500
Invisible GoAway
0
0


* -----------------  DIALOGS -------------------- *

TYPE DLOG
Tempo,534
Metronome;;
40 345 90 505
Invisible GoAway
0	;; Dialog def. ID
0	;; refCon
600	;; Dialog item list ID


TYPE DITL
TempoList,600
4

*   1
EditText Disabled
5 68 23 140

*   2
RadioItem Enabled
28 5 45 75
smooth

*   3
RadioItem Enabled
28 77 45 155
striated

*   4
StatText Enabled
5 13 23 61
mm =


TYPE DLOG
SettingsTop,535
Computation settings
38 0 100 640 ;; Button height is 14.
Invisible GoAway
16	;; Dialog def. ID
1	;; See note on my refCon's.  String list is ID=300
602 ;; Dialog item list ID (can't be blank on Mac II)

TYPE DITL
Dummy,602
1

staticText disabled ;; rubbish (DITL can't be blank)
0 0 1 1
$


TYPE DLOG
SettingsBottom,536
Input - output settings
425 0 480 640 ;; Button height is 14.
Invisible GoAway
16	;; Dialog def. ID
5	;; See note on my refCon's.  String list is ID=301
603	;; Dialog item list ID (blank)

TYPE DITL
Dummy,603
1

staticText disabled ;; rubbish (DITL can't be blank)
0 0 1 1
$


TYPE DLOG
RandomSequence,525
Random;;
94 345 151 505
Invisible GoAway
16	;; Dialog def. ID
0	;; refCon
614	;; Dialog item list ID


TYPE DITL
RandomSequenceList,614
4

button
32 3 50 73
Reset

button
32 78 50 157
New seed

editText disabled
5 68 23 140
 ;;

staticText enabled
5 13 23 61
Seed =



TYPE DLOG
Accuracy,526
Accuracy;;
171 378 340 508
Invisible GoAway
16	;; Dialog def. ID
0	;; refCon
615	;; Dialog item list ID


TYPE DITL
AccuracyList,615
11

*   1 = fQuantize
EditText Disabled
70 8 87 84

*   2 = dOff
RadioItem Enabled
94 3 112 43
OFF

*   3 = dOn
RadioItem Enabled
94 51 112 91
ON

*   4
StatText Disabled
73 89 91 112
ms

*   5
staticText enabled
50 2 67 98
Quantization:

*   6
staticText enabled
3 2 21 112
Time resolution:

*   7 = fTimeRes
EditText Enabled
24 8 41 84


*   8
StatText Disabled
24 89 42 112
ms

*   9
staticText enabled
121 2 141 126
MIDI set-up time:

*   10 = fSetUpTime
EditText Enabled
145 9 162 84


*   11
StatText Disabled
146 89 164 112
ms



TYPE DLOG
TimeBase,527
Time base;;
40 1 469 416
Invisible GoAway
0	;; Dialog def. ID
0	;; refCon
616	;; Dialog item list ID


Type DITL
TimeBaseList,616
204

*   1
EditText Enabled
10 5 27 75
1

*   2
EditText Enabled
10 136 27 206
1

*   3
StatText Enabled
10 82 27 131
ticks in

*   4
StatText Enabled
10 213 28 245
sec

*   5
BtnItem Enabled
3 325 31 412
Play ticks

*   6
StatText Enabled
93 4 110 37
key

*   7
EditText Enabled
93 35 109 61
96

*   8
StatText Enabled
93 65 109 84
ch

*   9
EditText Enabled
93 87 109 107
1

*   10
BtnItem Enabled
90 267 113 325
Rec tick

*   11
StatText Enabled
93 112 110 138
vel

*   12
EditText Enabled
93 138 109 163
64

*   13
ChkItem Enabled
131 3 148 20
Check Box

*   14
ChkItem Enabled
131 19 148 36
Check Box

*   15
ChkItem Enabled
131 35 148 52
Check Box

*   16
ChkItem Enabled
131 51 148 68
Check Box

*   17
ChkItem Enabled
131 67 148 84
Check Box

*   18
ChkItem Enabled
131 83 148 100
Check Box

*   19
ChkItem Enabled
131 99 148 116
Check Box

*   20
ChkItem Enabled
131 115 148 132
Check Box

*   21
ChkItem Enabled
131 131 148 148
Check Box

*   22
ChkItem Enabled
131 147 148 164
Check Box

*   23
ChkItem Enabled
131 163 148 180
Check Box

*   24
ChkItem Enabled
131 179 148 196
Check Box

*   25
ChkItem Enabled
131 195 148 212
Check Box

*   26
ChkItem Enabled
131 211 148 228
Check Box

*   27
ChkItem Enabled
131 227 148 244
Check Box

*   28
ChkItem Enabled
131 243 148 260
Check Box

*   29
ChkItem Enabled
131 259 148 276
Check Box

*   30
ChkItem Enabled
131 275 148 292
Check Box

*   31
ChkItem Enabled
131 291 148 308
Check Box

*   32
ChkItem Enabled
131 307 148 324
Check Box

*   33
ChkItem Enabled
147 3 164 20
Check Box

*   34
ChkItem Enabled
147 19 164 36
Check Box

*   35
ChkItem Enabled
147 35 164 52
Check Box

*   36
ChkItem Enabled
147 51 164 68
Check Box

*   37
ChkItem Enabled
147 67 164 84
Check Box

*   38
ChkItem Enabled
147 83 164 100
Check Box

*   39
ChkItem Enabled
147 99 164 116
Check Box

*   40
ChkItem Enabled
147 115 164 132
Check Box

*   41
ChkItem Enabled
147 131 164 148
Check Box

*   42
ChkItem Enabled
147 147 164 164
Check Box

*   43
ChkItem Enabled
147 163 164 180
Check Box

*   44
ChkItem Enabled
147 179 164 196
Check Box

*   45
ChkItem Enabled
147 195 164 212
Check Box

*   46
ChkItem Enabled
147 211 164 228
Check Box

*   47
ChkItem Enabled
147 227 164 244
Check Box

*   48
ChkItem Enabled
147 243 164 260
Check Box

*   49
ChkItem Enabled
147 259 164 276
Check Box

*   50
ChkItem Enabled
147 275 164 292
Check Box

*   51
ChkItem Enabled
147 291 164 308
Check Box

*   52
ChkItem Enabled
147 307 164 324
Check Box

*   53
StatText Disabled
116 5 131 20
1

*   54
StatText Disabled
116 70 131 85
5

*   55
StatText Disabled
116 145 131 165
10

*   56
StatText Disabled
116 225 131 245
15

*   57
StatText Disabled
116 306 131 326
20

*   58
StatText Enabled
66 114 83 175
Cycle of

*   59
EditText Enabled
66 176 82 205
4

*   60
StatText Enabled
65 210 82 327
beat(s) [max 40]

*   61
StatText Enabled
204 4 221 37
key

*   62
EditText Enabled
204 35 220 61
84

*   63
StatText Enabled
204 64 221 85
ch

*   64
EditText Enabled
204 87 220 106
1

*   65
BtnItem Enabled
201 266 224 324
Rec tick

*   66
StatText Enabled
204 111 220 135
vel

*   67
EditText Enabled
204 139 220 164
64

*   68
ChkItem Enabled
243 3 260 20
Check Box

*   69
ChkItem Enabled
243 19 260 36
Check Box

*   70
ChkItem Enabled
243 35 260 52
Check Box

*   71
ChkItem Enabled
243 51 260 68
Check Box

*   72
ChkItem Enabled
243 67 260 84
Check Box

*   73
ChkItem Enabled
243 83 260 100
Check Box

*   74
ChkItem Enabled
243 99 260 116
Check Box

*   75
ChkItem Enabled
243 115 260 132
Check Box

*   76
ChkItem Enabled
243 131 260 148
Check Box

*   77
ChkItem Enabled
243 147 260 164
Check Box

*   78
ChkItem Enabled
243 163 260 180
Check Box

*   79
ChkItem Enabled
243 179 260 196
Check Box

*   80
ChkItem Enabled
243 195 260 212
Check Box

*   81
ChkItem Enabled
243 211 260 228
Check Box

*   82
ChkItem Enabled
243 227 260 244
Check Box

*   83
ChkItem Enabled
243 243 260 260
Check Box

*   84
ChkItem Enabled
243 259 260 276
Check Box

*   85
ChkItem Enabled
243 275 260 292
Check Box

*   86
ChkItem Enabled
243 291 260 308
Check Box

*   87
ChkItem Enabled
243 307 260 324
Check Box

*   88
ChkItem Enabled
259 3 276 20
Check Box

*   89
ChkItem Enabled
259 19 276 36
Check Box

*   90
ChkItem Enabled
259 35 276 52
Check Box

*   91
ChkItem Enabled
259 51 276 68
Check Box

*   92
ChkItem Enabled
259 67 276 84
Check Box

*   93
ChkItem Enabled
259 83 276 100
Check Box

*   94
ChkItem Enabled
259 99 276 116
Check Box

*   95
ChkItem Enabled
259 115 276 132
Check Box

*   96
ChkItem Enabled
259 131 276 148
Check Box

*   97
ChkItem Enabled
259 147 276 164
Check Box

*   98
ChkItem Enabled
259 163 276 180
Check Box

*   99
ChkItem Enabled
259 179 276 196
Check Box

*   100
ChkItem Enabled
259 195 276 212
Check Box

*   101
ChkItem Enabled
259 211 276 228
Check Box

*   102
ChkItem Enabled
259 227 276 244
Check Box

*   103
ChkItem Enabled
259 243 276 260
Check Box

*   104
ChkItem Enabled
259 259 276 276
Check Box

*   105
ChkItem Enabled
259 275 276 292
Check Box

*   106
ChkItem Enabled
259 291 276 308
Check Box

*   107
ChkItem Enabled
259 307 276 324
Check Box

*   108
StatText Disabled
228 5 243 20
1

*   109
StatText Disabled
228 70 243 85
5

*   110
StatText Disabled
228 145 243 165
10

*   111
StatText Disabled
228 225 243 245
15

*   112
StatText Disabled
228 306 243 326
20

*   113
StatText Enabled
178 114 195 175
Cycle of

*   114
EditText Enabled
178 176 194 205
4

*   115
StatText Enabled
177 210 194 327
beat(s) [max 40]

*   116
StatText Enabled
326 4 343 37
key

*   117
EditText Enabled
326 33 342 59
72

*   118
StatText Enabled
326 63 342 82
ch

*   119
EditText Enabled
326 86 342 109
1

*   120
BtnItem Enabled
322 265 346 323
Rec tick

*   121
StatText Enabled
326 113 342 138
vel

*   122
EditText Enabled
326 141 342 166
64

*   123
ChkItem Enabled
361 3 378 20
Check Box

*   124
ChkItem Enabled
361 19 378 36
Check Box

*   125
ChkItem Enabled
361 35 378 52
Check Box

*   126
ChkItem Enabled
361 51 378 68
Check Box

*   127
ChkItem Enabled
361 67 378 84
Check Box

*   128
ChkItem Enabled
361 83 378 100
Check Box

*   129
ChkItem Enabled
361 99 378 116
Check Box

*   130
ChkItem Enabled
361 115 378 132
Check Box

*   131
ChkItem Enabled
361 131 378 148
Check Box

*   132
ChkItem Enabled
361 147 378 164
Check Box

*   133
ChkItem Enabled
361 163 378 180
Check Box

*   134
ChkItem Enabled
361 179 378 196
Check Box

*   135
ChkItem Enabled
361 195 378 212
Check Box

*   136
ChkItem Enabled
361 211 378 228
Check Box

*   137
ChkItem Enabled
361 227 378 244
Check Box

*   138
ChkItem Enabled
361 243 378 260
Check Box

*   139
ChkItem Enabled
361 259 378 276
Check Box

*   140
ChkItem Enabled
361 275 378 292
Check Box

*   141
ChkItem Enabled
361 291 378 308
Check Box

*   142
ChkItem Enabled
361 307 378 324
Check Box

*   143
ChkItem Enabled
377 3 394 20
Check Box

*   144
ChkItem Enabled
377 19 394 36
Check Box

*   145
ChkItem Enabled
377 35 394 52
Check Box

*   146
ChkItem Enabled
377 51 394 68
Check Box

*   147
ChkItem Enabled
377 67 394 84
Check Box

*   148
ChkItem Enabled
377 83 394 100
Check Box

*   149
ChkItem Enabled
377 99 394 116
Check Box

*   150
ChkItem Enabled
377 115 394 132
Check Box

*   151
ChkItem Enabled
377 131 394 148
Check Box

*   152
ChkItem Enabled
377 147 394 164
Check Box

*   153
ChkItem Enabled
377 163 394 180
Check Box

*   154
ChkItem Enabled
377 179 394 196
Check Box

*   155
ChkItem Enabled
377 195 394 212
Check Box

*   156
ChkItem Enabled
377 211 394 228
Check Box

*   157
ChkItem Enabled
377 227 394 244
Check Box

*   158
ChkItem Enabled
377 243 394 260
Check Box

*   159
ChkItem Enabled
377 259 394 276
Check Box

*   160
ChkItem Enabled
377 275 394 292
Check Box

*   161
ChkItem Enabled
377 291 394 308
Check Box

*   162
ChkItem Enabled
377 307 394 324
Check Box

*   163
StatText Disabled
346 5 361 20
1

*   164
StatText Disabled
346 70 361 85
5

*   165
StatText Disabled
346 145 361 165
10

*   166
StatText Disabled
346 225 361 245
15

*   167
StatText Disabled
346 306 361 326
20

*   168
StatText Enabled
296 114 313 175
Cycle of

*   169
EditText Enabled
296 176 312 205
4

*   170
StatText Enabled
295 210 312 327
beat(s) [max 40]

*   171
BtnItem Enabled
36 334 57 413
Reset cycle

*   172
BtnItem Enabled
64 5 84 92
Clear cycle

*   173
BtnItem Enabled
175 5 195 92
Clear cycle

*   174
BtnItem Enabled
294 5 314 92
Clear cycle

*   175
BtnItem Enabled
36 6 56 64
NEW

*   176
BtnItem Enabled
36 66 56 124
LOAD

*   177
BtnItem Enabled
36 126 56 184
SAVE

*   178
BtnItem Enabled
64 341 84 399
MUTE

*   179
EditText Enabled
117 348 135 404
1

*   180
EditText Enabled
144 348 161 404
1

*   181
StatText Disabled
144 330 160 345
/

*   182
StatText Enabled
93 331 110 414
Speed ratio:

*   183
BtnItem Enabled
177 340 197 398
MUTE

*   184
EditText Enabled
230 347 248 404
1

*   185
EditText Enabled
257 347 274 404
1

*   186
StatText Disabled
257 329 273 344
/

*   187
StatText Enabled
204 330 221 413
Speed ratio:

*   188
BtnItem Enabled
295 340 315 398
MUTE

*   189
EditText Enabled
348 347 366 404
1

*   190
EditText Enabled
375 347 392 404
1

*   191
StatText Disabled
375 329 391 344
/

*   192
StatText Enabled
326 330 343 413
Speed ratio:

*   193
BtnItem Enabled
36 186 56 260
SAVE AS...

*   194
StatText Enabled
93 168 110 194
dur

*   195
EditText Enabled
93 197 109 234
50

*   196
StatText Enabled
93 239 110 264
ms

*   197
StatText Enabled
204 169 221 194
dur

*   198
EditText Enabled
204 198 220 235
50

*   199
StatText Enabled
203 240 221 262
ms

*   200
StatText Enabled
326 170 342 195
dur

*   201
EditText Enabled
326 198 342 235
50

*   202
StatText Enabled
326 240 342 263
ms

*   203
EditText Enabled
402 8 421 404
(Comment)

*   204
BtnItem Enabled
36 262 57 332
Capture…


TYPE DLOG
BufferSize,528
Buffer size;;
40 10 115 170
Invisible GoAway
16	;; Dialog def. ID
0	;; refCon
617	;; Dialog item list ID


TYPE DITL
BufferSizeList,617
6

editText disabled
5 12 23 72
 ;;

radioButton enabled
28 5 45 90
don’t stop

radioButton enabled
28 92 45 160
stop

staticText disabled
5 90 23 150
symbols

editText disabled
52 90 70 150
 ;;

staticText disabled
52 12 70 85
Default:



Type DLOG
FindReplace,529
Find;;
220 40 322 468
Invisible GoAway
0	;; Dialog def.ID
0	;; refCon
618 ;; Dialog item list ID

Type DITL
FindReplaceList,618
9

*   1
BtnItem Enabled
78 150 98 208
FIND

*   2  Find field
EditText Enabled
10 130 33 420
 ;;

*   3  Replace field
EditText Enabled
40 130 66 420
 ;;

*   4
staticText enabled
10 10 28 121
Find / Replace:

*   5
StatText Disabled
40 46 56 121
with:

*   6
BtnItem Enabled
78 360 98 418
CANCEL

*   7
BtnItem Enabled
78 230 98 334
REPLACE ALL

*   8
ChkItem Enabled
60 10 78 116
Ignore case

*   9
ChkItem Enabled
80 10 98 116
Match words


Type DLOG
GraphicSettings,530
Graphics;;
76 184 152 450
Invisible GoAway
16	;; Dialog def. ID
0	;; refCon
622	;; Dialog item list ID


Type DITL
GraphicSettingsList,622
6

*   1
EditText Disabled
14 102 32 186

*   2
staticText enabled
5 16 41 94
500 pixels represent:

*   3
StatText Disabled
15 196 31 255
seconds

*   4
ChkItem Enabled
38 175 55 207
0

*   5
ChkItem Enabled
55 175 72 207
1

*   6
staticText enabled
47 16 65 174
Number streaks from:


TYPE DLOG
ControlPanel,531
Control panel
56 304 422 510
Invisible GoAway
16
0	;; refCon
601	;; Dialog item list ID


TYPE DITL
ControlPanelList,601
23

*   1
BtnItem Enabled
4 3 40 60
Produce\0Ditems

*   2
BtnItem Enabled
127 114 166 202
Derive\0Dfurther

*   3
BtnItem Enabled
170 2 196 101
Load decisions

*   4
BtnItem Enabled
170 104 196 205
Save decisions

*   5
BtnItem Enabled
126 4 166 101
Repeat\0Dproduction

*   6
BtnItem Enabled
201 72 235 134
Show\0Dweights

*   7
BtnItem Enabled
326 4 362 61
MIDI\0Dcheck

*   8
BtnItem Enabled
4 66 40 130
Play\0Dselection

*   9
BtnItem Enabled
4 136 40 202
Expand\0Dselection

*   10
BtnItem Enabled
86 114 123 202
Produce\0Dtemplates

*   11
BtnItem Enabled
88 4 124 100
Analyze\0Dselection

*   12
BtnItem Enabled
201 139 235 204
Load\0Dweights

*   13
BtnItem Enabled
200 4 235 65
Learn\0Dweights

*   14
BtnItem Enabled
238 72 274 135
Save\0Dweights

*   15
BtnItem Enabled
239 4 274 66
Set\0Dweights

*   16
BtnItem Enabled
326 66 362 123
MIDI\0DPANIC

*   17
BtnItem Enabled
44 112 83 202
Show\0Dperiods

*   18
ChkItem Enabled
278 5 298 170
Transpose input notes

*   19
EditText Enabled
303 10 319 47
0

*   20
StatText Disabled
302 51 319 153
semitones (up)

*   21
BtnItem Enabled
43 4 85 99
Capture\0Dselection as…

*   22
BtnItem Enabled
326 127 362 204
Reset\0Dcontrollers

*   23
BtnItem Enabled
299 156 324 203
Mute


TYPE DLOG
Keyboard,532
New keyboard
40 2 359 481
Invisible GoAway
16 ;; ProcId
0	;; refCon
626	;; Dialog item list ID

TYPE DITL
KeyboardList,626
109

*   1
BtnItem Enabled
290 410 310 468
OK

*   2
BtnItem Enabled
290 250 310 308
LOAD

*   3
BtnItem Enabled
290 320 310 378
SAVE

*   4
StatText Disabled
10 10 25 39
A = 

*   5
StatText Disabled
30 10 45 39
B = 

*   6
StatText Disabled
50 10 65 39
C = 

*   7
StatText Disabled
70 10 85 39
D = 

*   8
StatText Disabled
90 10 105 39
E = 

*   9
StatText Disabled
110 10 125 39
F = 

*   10
StatText Disabled
130 10 145 39
G = 

*   11
StatText Disabled
150 10 165 39
H = 

*   12
StatText Disabled
170 10 185 39
I = 

*   13
StatText Disabled
190 10 205 39
J = 

*   14
StatText Disabled
210 10 225 39
K = 

*   15
StatText Disabled
230 10 245 39
L = 

*   16
StatText Disabled
250 10 265 39
M = 

*   17
StatText Disabled
10 120 25 149
N = 

*   18
StatText Disabled
30 120 45 149
O = 

*   19
StatText Disabled
50 120 65 149
P = 

*   20
StatText Disabled
70 120 85 149
Q = 

*   21
StatText Disabled
90 120 105 149
R = 

*   22
StatText Disabled
110 120 125 149
S = 

*   23
StatText Disabled
130 120 145 149
T = 

*   24
StatText Disabled
150 120 165 149
U = 

*   25
StatText Disabled
170 120 185 149
V = 

*   26
StatText Disabled
190 120 205 149
W = 

*   27
StatText Disabled
210 120 225 149
X = 

*   28
StatText Disabled
230 120 245 149
Y = 

*   29
StatText Disabled
250 120 265 149
Z = 

*   30
EditText Enabled
10 40 25 113
A

*   31
EditText Enabled
30 40 45 113
B

*   32
EditText Enabled
50 40 65 113
C

*   33
EditText Enabled
70 40 85 113
D

*   34
EditText Enabled
90 40 105 113
E

*   35
EditText Enabled
110 40 125 113
F

*   36
EditText Enabled
130 40 145 113
G

*   37
EditText Enabled
150 40 165 113
H

*   38
EditText Enabled
170 40 185 113
I

*   39
EditText Enabled
190 40 205 113
J

*   40
EditText Enabled
210 40 225 113
K

*   41
EditText Enabled
230 40 245 113
L

*   42
EditText Enabled
250 40 265 113
M

*   43
EditText Enabled
10 150 25 223
N

*   44
EditText Enabled
30 150 45 223
O

*   45
EditText Enabled
50 150 65 223
P

*   46
EditText Enabled
70 150 85 223
Q

*   47
EditText Enabled
90 150 105 223
R

*   48
EditText Enabled
110 150 125 223
S

*   49
EditText Enabled
130 150 145 223
T

*   50
EditText Enabled
150 150 165 223
U

*   51
EditText Enabled
170 150 185 223
V

*   52
EditText Enabled
190 150 205 223
W

*   53
EditText Enabled
210 150 225 223
X

*   54
EditText Enabled
230 150 245 223
Y

*   55
EditText Enabled
250 150 265 223
Z

*   56
StatText Disabled
10 250 25 279
a = 

*   57
StatText Disabled
30 250 45 279
b = 

*   58
StatText Disabled
50 250 65 279
c = 

*   59
StatText Disabled
70 250 85 279
d = 

*   60
StatText Disabled
90 250 105 279
e = 

*   61
StatText Disabled
110 250 125 279
f = 

*   62
StatText Disabled
130 250 145 279
g = 

*   63
StatText Disabled
150 250 165 279
h = 

*   64
StatText Disabled
170 250 185 279
i = 

*   65
StatText Disabled
190 250 205 279
j = 

*   66
StatText Disabled
210 250 225 279
k = 

*   67
StatText Disabled
230 250 245 279
l = 

*   68
StatText Disabled
250 250 265 279
m = 

*   69
StatText Disabled
10 360 25 389
n = 

*   70
StatText Disabled
30 360 45 389
o =

*   71
StatText Disabled
50 360 65 389
p = 

*   72
StatText Disabled
70 360 85 389
q = 

*   73
StatText Disabled
90 360 105 389
r = 

*   74
StatText Disabled
110 360 125 389
s = 

*   75
StatText Disabled
130 360 145 389
t = 

*   76
StatText Disabled
150 360 165 389
u = 

*   77
StatText Disabled
170 360 185 389
v = 

*   78
StatText Disabled
190 360 205 389
w = 

*   79
StatText Disabled
210 360 225 389
x = 

*   80
StatText Disabled
230 360 245 389
y = 

*   81
StatText Disabled
250 360 265 389
z = 

*   82
EditText Enabled
10 280 25 353
.

*   83
EditText Enabled
30 280 45 353
.

*   84
EditText Enabled
50 280 65 353
.

*   85
EditText Enabled
70 280 85 353
.

*   86
EditText Enabled
90 280 105 353
.

*   87
EditText Enabled
110 280 125 353
.

*   88
EditText Enabled
130 280 145 353
.

*   89
EditText Enabled
150 280 165 353
.

*   90
EditText Enabled
170 280 185 353
.

*   91
EditText Enabled
190 280 205 353
.

*   92
EditText Enabled
210 280 225 353
.

*   93
EditText Enabled
230 280 245 353
.

*   94
EditText Enabled
250 280 265 353
.

*   95
EditText Enabled
10 390 25 463
.

*   96
EditText Enabled
30 390 45 463
.

*   97
EditText Enabled
50 390 65 463
.

*   98
EditText Enabled
70 390 85 463
.

*   99
EditText Enabled
90 390 105 463
.

*   100
EditText Enabled
110 390 125 463
.

*   101
EditText Enabled
130 390 145 463
.

*   102
EditText Enabled
150 390 165 463
.

*   103
EditText Enabled
170 390 185 463
.

*   104
EditText Enabled
190 390 205 463
.

*   105
EditText Enabled
210 390 225 463
.

*   106
EditText Enabled
230 390 245 463
.

*   107
EditText Enabled
250 390 265 463
.

*   108
RadioItem Enabled
290 10 309 146
Use these tokens

*   109
BtnItem Enabled
290 160 310 218
RESET


Type DLOG
ScriptDialog,533
Script
42 260 98 510
Invisible GoAway
16  ;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
629 ;; Dialog item list ID


Type DITL
ScriptDialogList,629
7

*   1
BtnItem Enabled
29 188 49 247
Execute

*   2
BtnItem Enabled
5 128 25 184
Save

*   3
BtnItem Enabled
5 188 25 246
Load

*   4
BtnItem Enabled
5 68 25 124
Clear

*   5
BtnItem Enabled
29 68 49 113
REC

*   6
BtnItem Enabled
29 115 49 184
Wait for:

*   7
BtnItem Enabled
6 5 49 65
Check\0Dscript


****  This is the end of dialogs treated as windows ****

TYPE DLOG
ButtonChart,550
 ;;
28 12 479 546
Invisible NoGoAway
1	;; Dialog def. ID (= dBoxProc)
0	;; refCon
604 ;; Dialog item list ID (blank): buttons are drawn by program

TYPE DITL
Dummy,604
1

staticText disabled ;; rubbish (DITL can't be blank)
0 0 1 1
$



Type DLOG
ResumeStop,551
 ;;
22 0 49 179
Invisible 2 NoGoAway
2	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
628 ;; Dialog item list ID


Type DITL
ResumeStopList,628
2

*   1
BtnItem Enabled
4 4 24 89
Resume…

*   2
BtnItem Enabled
4 96 24 176
STOP


TYPE DLOG
ReplaceCommand,552
 ;;
16 0 43 315
Invisible NoGoAway
2	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
619 ;; Dialog item list ID

TYPE DITL
ReplaceCancelList,619
3

BtnItem Enabled
4 110 24 208
Find next

BtnItem Enabled
4 5 24 90
Replace

BtnItem Enabled
4 230 24 310
STOP


Type DLOG
MIDIsettings,553
 ;;
80 80 230 400
Invisible NoGoAway
2	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
621 ;; Dialog item list ID

Type DITL
MIDIsettingslist,621
15

*   1
BtnItem Enabled
117 250 137 308
OK

*   2
BtnItem Enabled
116 18 137 144
Reset to default

*   3
BtnItem Enabled
117 167 137 225
Cancel

*   4
staticText enabled
6 26 23 161
Size of MIDI buffer:

*   5
StatText Disabled
7 234 23 287
kbytes

*   6
EditText Enabled
8 169 23 222


*   7
staticText enabled
28 26 44 155
Max storage time:

*   8
StatText Disabled
29 232 44 290
seconds

*   9
EditText Enabled
30 169 44 222


*   10
staticText enabled
69 26 85 101
MIDI port:

*   11
RadioItem Enabled
56 103 74 171
Printer

*   12
RadioItem Enabled
81 103 100 176
Modem

*   13
EditText Enabled
68 242 85 263


*   14
staticText enabled
70 194 85 237
Clock:

*   15
StatText Disabled
70 270 85 306
MHz


Type DLOG
ResumeUndoStop,554
 ;;
16 0 43 270
Invisible NoGoAway
2	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
623 ;; Dialog item list ID

Type DITL
ResumeUndoStopList,623
3

*   1
BtnItem Enabled
5 93 25 178
Resume…

*   2
BtnItem Enabled
5 183 25 263
STOP

*   3
BtnItem Enabled
5 6 25 88
Undo


Type DLOG
Pattern,555
 ;;
86 108 189 368
Invisible NoGoAway
1	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
625 ;; Dialog item list ID

Type DITL
PatternList,625
7

*   1
BtnItem Enabled
64 180 91 256
OK

*   2
staticText enabled
4 4 26 134
Name of pattern:

*   3
EditText Enabled
4 140 26 253
Tp1

*   4
staticText enabled
32 4 50 131
Symbolic duration:

*   5
EditText Enabled
32 140 52 208
(integer)

*   6
StatText Disabled
32 213 49 256
beats

*   7
BtnItem Enabled
64 4 92 69
Cancel



Type DLOG
Enter,556
 ;;
86 108 164 370
Invisible NoGoAway
1	;; Dialog def. ID (= plainDBoxProc)
0	;; refCon
627 ;; Dialog item list ID


Type DITL
EnterList,627
4

*   1
BtnItem Enabled
48 180 75 256
OK

*   2
BtnItem Enabled
48 4 76 69
Cancel

*   3
EditText Enabled
20 188 37 253
 ;;

*   4
StatText Disabled
4 4 42 181
 ;;


Type DLOG
NoteConvention,557
Note convention
40 40 388 289
Invisible GoAway
4
0
632

Type DITL
NoteConventionList,632
27

*   1
RadioItem Enabled
246 121 264 178
do

*   2
RadioItem Enabled
226 121 244 178
do#

*   3
RadioItem Enabled
206 121 224 178
re

*   4
RadioItem Enabled
186 121 204 178
re#

*   5
RadioItem Enabled
166 121 184 178
mi

*   6
RadioItem Enabled
130 120 148 177
fa

*   7
RadioItem Enabled
110 120 128 177
fa#

*   8
RadioItem Enabled
90 120 108 177
sol

*   9
RadioItem Enabled
70 120 88 177
sol#

*   10
RadioItem Enabled
50 120 68 177
la

*   11
RadioItem Enabled
30 120 48 177
la#

*   12
RadioItem Enabled
10 120 28 177
si

*   13
RadioItem Enabled
246 181 264 238
si#

*   14
RadioItem Enabled
226 181 244 238
reb

*   15
RadioItem Disabled
206 181 224 238
re

*   16
RadioItem Enabled
186 181 204 238
mib

*   17
RadioItem Enabled
166 181 184 238
fab

*   18
RadioItem Enabled
130 180 148 237
mi#

*   19
RadioItem Enabled
110 180 128 237
solb

*   20
RadioItem Disabled
90 180 108 237
sol

*   21
RadioItem Enabled
70 180 88 237
lab

*   22
RadioItem Disabled
50 180 68 237
la

*   23
RadioItem Enabled
30 180 48 237
sib

*   24
RadioItem Enabled
10 180 28 237
dob

*   25
staticText enabled
285 5 335 180
All names are recognized. The checked ones are used for the display.

*   26
BtnItem Enabled
289 186 334 243
OK

*   27
PicItem Disabled
3 5 274 113
128


Type DLOG
TickDialog,545
Tick settings
58 452 289 635
Invisible NoGoAway
3
0
639

Type DITL
TickDialogList,639
13

*   1
BtnItem Enabled
200 140 223 176
OK

*   2
StatText Enabled
10 10 30 100
Tick settings

*   3
RadioItem Enabled
10 130 28 173
ON

*   4
RadioItem Enabled
30 130 47 176
OFF

*   5
RadioItem Enabled
60 10 79 171
Use default settings

*   6
RadioItem Enabled
80 10 101 177
Use special settings...

*   7
EditText Enabled
110 130 127 167
1

*   8
EditText Enabled
140 130 158 168
60

*   9
EditText Enabled
170 130 188 168
127

*   10
StatText Disabled
110 30 126 105
Channel

*   11
StatText Disabled
140 30 156 105
Key

*   12
StatText Disabled
170 30 186 105
Velocity

*   13
BtnItem Enabled
200 10 224 130
Capture settings



Type DLOG
MIDIfilter,544
MIDI filter
40 40 412 230
Invisible GoAway
16
0
638

Type DITL
MIDIfilterlist,638
48

*   1
ChkItem Enabled
36 5 54 22

*   2
ChkItem Enabled
56 5 74 22

*   3
ChkItem Enabled
76 5 94 22

*   4
ChkItem Enabled
96 5 114 22

*   5
ChkItem Enabled
116 5 134 22

*   6
ChkItem Enabled
136 5 154 22

*   7
ChkItem Enabled
156 5 174 22

*   8
ChkItem Enabled
176 5 194 22

*   9
ChkItem Enabled
196 5 214 22

*   10
ChkItem Enabled
216 5 234 22

*   11
ChkItem Enabled
236 5 254 22

*   12
ChkItem Enabled
256 5 274 22

*   13
ChkItem Enabled
276 5 294 22

*   14
ChkItem Enabled
296 5 314 22

*   15
ChkItem Enabled
316 5 334 22

*   16
ChkItem Enabled
36 170 54 187

*   17
ChkItem Enabled
56 170 74 187

*   18
ChkItem Enabled
76 170 94 187

*   19
ChkItem Enabled
96 170 114 187

*   20
ChkItem Enabled
116 170 134 187

*   21
ChkItem Enabled
136 170 154 187

*   22
ChkItem Enabled
156 170 174 187

*   23
ChkItem Enabled
176 170 194 187

*   24
ChkItem Enabled
196 170 214 187

*   25
ChkItem Enabled
216 170 234 187

*   26
ChkItem Enabled
236 170 254 187

*   27
ChkItem Enabled
256 170 274 187

*   28
ChkItem Enabled
276 170 294 187

*   29
ChkItem Enabled
296 170 314 187

*   30
ChkItem Enabled
316 170 334 187

*   31
staticText enabled
12 5 29 76
RECEIVE...

*   32
staticText enabled
12 108 27 191
TRANSMIT...

*   33
StatText Disabled
37 37 52 153
NoteOn/NoteOff

*   34
StatText Disabled
57 49 74 141
Key pressure

*   35
StatText Disabled
78 68 94 122
Control

*   36
StatText Disabled
97 39 115 151
Program change

*   37
StatText Disabled
117 35 134 156
Channel pressure

*   38
StatText Disabled
138 58 154 133
Pitchbend

*   39
StatText Disabled
158 33 176 158
System exclusive

*   40
StatText Disabled
177 58 193 133
Time code

*   41
StatText Disabled
197 48 215 143
Song position

*   42
StatText Disabled
217 56 234 135
Song select

*   43
StatText Disabled
237 50 254 140
Tune request

*   44
StatText Disabled
258 53 275 138
Timing clock

*   45
StatText Disabled
278 27 296 163
Start/continue/stop

*   46
StatText Disabled
298 45 315 145
Active sensing

*   47
StatText Disabled
318 49 336 141
System reset

*   48
BtnItem Enabled
343 66 363 124
Reset


Type DLOG
CsoundTables,548
Csound tables
44 10 470 316
Invisible GoAway
16
0
649

Type DITL
CsoundTablesList,649
1

*   1
BtnItem Enabled
400 243 420 301
OK


* -----------------  ALERTS -------------------- *

Type ALRT
StepAlert,142
30 394 158 500
611
4444 ;; Stages word.  See Inside Mac I-426.

Type DITL
StepAlertList,611
4
*   1
BtnItem Enabled
96 7 116 95
Accept

*   2
BtnItem Enabled
63 6 83 94
Step

*   3
BtnItem Enabled
34 6 54 94
Next sol

*   4
BtnItem Enabled
5 5 25 93
Abort


TYPE ALRT
OKAlert,143
76 118 190 382
612
5555

TYPE DITL
OKAlertList,612
2
*   1
button Enabled
83 198 103 258
OK

*   2
StaticText Enabled
3 60 75 258
^0^1^2^3


Type ALRT
YesNo,144
71 112 167 413
605
4444

Type DITL
YesNoList,605
3

*   1
BtnItem Enabled
62 211 86 297
Yes

*   2
BtnItem Enabled
62 108 86 194
No

*   3
StatText Disabled
2 5 56 297
^0^1^2^3


Type ALRT
NoYes,145
71 112 167 413
606
4444

Type DITL
NoYesList,606
3

*   1
BtnItem Enabled
62 209 86 295
No

*   2
BtnItem Enabled
62 108 86 194
Yes

*   3
StatText Disabled
2 5 56 297
^0^1^2^3


Type ALRT
YesNoCancel,146
71 112 167 413
607
4444

Type DITL
YesNoCancelList,607
4

*   1
BtnItem Enabled
62 211 86 297
Yes

*   2
BtnItem Enabled
62 108 86 194
No

*   3
BtnItem Enabled
62 5 86 91
Cancel

*   4
StatText Disabled
2 5 56 297
^0^1^2^3


Type ALRT
NoYesCancel,147
71 112 167 413
608
4444

Type DITL
NoYesCancelList,608

4
*   1
BtnItem Enabled
62 209 86 295
No

*   2
BtnItem Enabled
62 108 86 194
Yes

*   3
BtnItem Enabled
62 5 86 91
Cancel

*   4
StatText Disabled
2 5 56 297
^0^1^2^3


Type ALRT
SaveAsAlert,149
71 146 189 362
620
4444

Type DITL
SaveAsOptions,620
5

*   1
BtnItem Enabled
42 119 64 200
BP2 file

*   2
BtnItem Enabled
42 15 64 96
Plain text

*   3
BtnItem Enabled
80 119 102 200
HTML (BP2)

*   4
StatText Disabled
8 78 25 138
Save as:

*   5
BtnItem Enabled
80 15 102 96
HTML (text)


Type ALRT
LoadNotes,150
71 146 152 362
624
4444

Type DITL
LoadNotesList,624
4

*   1
BtnItem Enabled
44 9 66 90
Cancel

*   2
BtnItem Enabled
44 102 66 211
MIDI device

*   3
BtnItem Enabled
8 130 30 211
From file

*   4
staticText enabled
9 9 29 99
Load notes:


Type ALRT
ColorAlert,151
38 222 331 492
630
5555


Type DITL
ColorAlertList,630
23
*   1
BtnItem Enabled
6 204 33 262
OK

*   2
BtnItem Enabled
48 12 66 30

*   3
StatText Enabled
48 36 66 170
Variables:  X, Var...

*   4
BtnItem Enabled
72 12 90 30

*   5
StatText Enabled
72 36 90 242
Terminal symbols:  a, <<a>> ...

*   6
BtnItem Enabled
96 12 114 30

*   7
StatText Enabled
96 36 114 226
Homomorphisms: *,  TRA ...

*   8
BtnItem Enabled
120 12 138 30

*   9
StatText Enabled
120 36 138 211
Time patterns: t1, t23 ...

*   10
BtnItem Enabled
144 12 162 30

*   11
StatText Enabled
144 36 162 211
Flags: /flag1/, /flag2/ ...

*   12
BtnItem Enabled
168 12 186 30

*   13
StatText Enabled
168 36 188 234
Controls: <K1>, <K2=x> ...

*   14
BtnItem Enabled
192 12 210 30

*   15
StatText Enabled
192 36 210 234
Sync tags: <<W1>>, <<W2>> ...

*   16
BtnItem Enabled
263 11 284 110
Sound objects

*   17
BtnItem Enabled
263 125 283 183
Streaks

*   18
BtnItem Enabled
263 191 283 249
Pivots

*   19
staticText enabled
6 12 24 189
Change display color in...

*   20
staticText enabled
24 12 40 87
TEXT:

*   21
staticText enabled
245 11 262 78
GRAPHICS:

*   22
BtnItem Enabled
216 12 236 30

*   23
StatText Enabled
219 36 236 132
Simple notes


Type ALRT
DisplayItemAlert,152
71 162 174 355
631
5555

Type DITL
DisplayItemList,631
4

*   1
BtnItem Enabled
12 127 62 184
Produce\0Dmore

*   2
BtnItem Enabled
12 6 62 114
Play again

*   3
BtnItem Enabled
72 6 92 114
Display item

*   4
BtnItem Enabled
72 126 92 184
Cancel

* -----------------  STRINGS -------------------- *

TYPE STR# ;; Settings top dialog window
SettingsTopDialog,300
16
Non-stop improvize
Cyclic play
Use each substitution
Produce all items
Display production
Step-by-step produce
Step subgrammars
Trace production
Choose candidate rule
Display items
Show graphics
Allow randomize
Time setting display
Time setting step
Time setting trace
Trace Csound


TYPE STR# ;; Settings bottom dialog window
SettingsBottomDialog,301
12
Use MIDI in/out
Synchronize start
Compute while playing
Interactive (use input)
Reset rule weights
Reset rule flags
Reset controllers
Ignore constraints
Write MIDI files
Show messages
Write Csound scores
Opcode OMS


TYPE STR# ;; All strings in program (Useless! should be suppressed…)
Misc,302
18
Cannot open MIDI driver.  Error;; 0
MIDI command: Stop repeating!;; 1
MIDI command: Abort!;; 2
MIDI command: Quick!;; 3
MIDI command: Play forever! (Key;; 4
channel;; 5
will stop.);; 6
Playing;; 7
times… (Key;; 8
MIDI command: Again!;; 9
MIDI command: Derive further!;; 10
Reset weights:;; 11
Play each substitution:;; 12
Synchronize start:;; 13
Time allocated:;; 14
Striated time:;; 15
Ignore constraints:;; 16
Skip next item;; 17


TYPE STR# ;; Script commands
Script,303
193
110 BP2 script
144 Define «variable» «sound item»
171 (-
42 Activate window «windowname»
167 AE send fast class '«AEclass»' ID '«AEID»' to application '«signature»'
168 AE send normal class '«AEclass»' ID '«AEID»' to application '«signature»'
169 AE wait class '«AEclass»' ID '«AEID»'
28 Alphabet
12 Analyze selection in window «windowname»
125 Autorandomize OFF
126 Autorandomize ON
148 Beep
48 Buffer size «long» symbols
64 Clear
25 Clear window «windowname»
9 Compile
111 Compute while play OFF
112 Compute while play ON
36 Control panel
62 Copy
23 Csound score OFF
22 Csound score ON
187 Csound trace OFF
186 Csound trace ON
115 Cyclic play OFF
116 Cyclic play ON
61 Cut
29 Data
73 Default buffer size «long» symbols
19 Delete «unsigned» chars
121 Display items OFF
122 Display items ON
152 Display time setting OFF
153 Display time setting ON
0 Expand selection window «windowname»
41 Freeze windows OFF
40 Freeze windows ON
143 Glossary
27 Grammar
163 Graphic Color OFF
162 Graphic Color ON
35 Graphics
51 Graphic scale «long» pixels = «long» milliseconds
158 Hide window «windowname»
137 Ignore constraints OFF
138 Ignore constraints ON
109 Info
31 Interaction
131 Interactive OFF
132 Interactive ON
38 Keyboard
5 Load Project «filename»
26 Load settings «filename»
18 Maximum production time «long» ticks
179 Metronom
185 MIDI file OFF
184 MIDI file ON
21 MIDI sound OFF
20 MIDI sound ON
30 Settings
4 New Project
113 Non-stop improvize OFF
114 Non-stop improvize ON
2 Note convention = English
1 Note convention = French
88 Note convention = Indian
3 Note convention = key numbers
49 Number streaks from 0
50 Number streaks from 1
33 Object prototypes
183 Opcode OMS OFF
182 Opcode OMS ON
24 Open file «filename»
63 Paste
69 Pause
13 Play selection in window «windowname»
85 Play «any item»
151 Play-show «int» times selection in window «windowname»
7 Print window «windowname»
119 Produce all items OFF
120 Produce all items ON
10 Produce items
147 Produce and play «int» items
11 Produce templates
149 Prompt ON
150 Prompt OFF
65 Quantization «long» milliseconds
72 Quantize OFF
52 Quantize ON
8 Quit
56 Randomize
174 Reset session time
107 Reset interaction
53 Reset keyboard
55 Reset random sequence
141 Reset controllers OFF
142 Reset controllers ON
135 Reset rule flags OFF
136 Reset rule flags ON
133 Reset rule weights OFF
134 Reset rule weights ON
176 Reset tick cycle
68 Return
74 Run script «filename»
108 Scrap
37 Script
6 Select all in window «windowname»
70 Set directory «long»
180 Set output Csound file «filename»
181 Set output MIDI file «filename»
164 Set output window «windowname»
57 Set random seed «unsigned»
59 Set selection start «long»
60 Set selection end «long»
39 Set Vref «int»
123 Show graphics OFF
124 Show graphics ON
139 Show messages OFF
140 Show messages ON
43 Smooth time
44 Striated time
32 Start string
170 Stop
129 Synchronize start OFF
130 Synchronize start ON
175 Tell session time
45 Tempo «long» ticks in «long» secs
160 Text Color OFF
159 Text Color ON
166 Tick cycle OFF
165 Tick cycle ON
173 Time base
177 Time ticks OFF
178 Time ticks ON
54 Time resolution «long» milliseconds
154 Time setting step OFF
155 Time setting step ON
156 Time setting trace OFF
157 Time setting trace ON
34 Trace
16 Type «line»
47 Use buffer limit OFF
46 Use buffer limit ON
117 Use each substitution OFF
118 Use each substitution ON
127 Use MIDI OFF
128 Use MIDI ON
67 Use tokens OFF
66 Use tokens ON
17 Wait «long» milliseconds
14 Wait for «key stroke or MIDI event»
145 (-
78 MIDI all notes off channel «1..16»
75 MIDI controller #«0..127» = «0..127» channel «1..16»
83 MIDI decimal send «decimal data»
84 MIDI hexa send «hexadecimal data»
76 MIDI local control OFF channel «1..16»
77 MIDI local control ON channel «1..16»
81 MIDI Mono mode ON [«0..16» voices] channel «1..16»
79 MIDI Omni mode OFF channel «1..16»
80 MIDI Omni mode ON channel «1..16»
82 MIDI Poly mode ON channel «1..16»
71 MIDI program «1..128» [basic channel]
190 MIDI send Start
191 MIDI send Continue
192 MIDI send Stop
146 MIDI set basic channel to «1..16»
58 MIDI set-up time «int» milliseconds
86 MIDI switch ON «64..95» channel «1..16»
87 MIDI switch OFF «64..95» channel «1..16»
172 (-
89 IN Derive further «note» channel «1..16» [toggle]
93 IN End repeat «note» channel «1..16»
106 IN Min «long» tick in «long» s «note» max «long» tick in «long» s «note» channel «1..16»
188 IN Mute ON «note» channel «1..16»
189 IN Mute OFF «note» channel «1..16»
161 IN On «note» channel «1..16» do «script instruction»
105 IN Param «Kx» = controller #«0..127» channel «1..16»
15 IN Parameter «Kx» = velocity «note» channel «1..16»
103 IN Play again item «note» channel «1..16»
95 IN Quit «note» channel «1..16»
94 IN Repeat forever «note» channel «1..16»
92 IN Repeat ‘v’ times «note» channel «1..16» [v = velocity]
90 IN Reset weights «note» channel «1..16» [toggle]
99 IN Set computation time to ‘v’ «note» channel «1..16» [v = velocity]
98 IN Set tempo controller #«0..127» channel «1..16» range «float»
102 IN Skip next item «note» channel «1..16»
100 IN Smooth-striated time «note» channel «1..16» [toggle]
91 IN Start play «note» channel «1..16»
104 IN Synchro tag «Wx» = «note» channel «1..16»
97 IN Synchronize start ON-OFF «note» channel «1..16» [toggle]
96 IN Use each substitution «note» channel «1..16» [toggle]
101 IN Use-ignore object constraints «note» channel «1..16» [toggle]


TYPE STR# ;; Grammar procedures
GrammarProcedures,304
17
11 2 _goto
0 2 _failed
1 1 _repeat
13 0 _stop
4 0 _print
6 0 _printOn
5 0 _printOff
12 0 _stepOn
2 0 _stepOff
15 0 _traceOn
16 0 _traceOff
8 0 _destru
7 0 _randomize
3 1 _mm
14 0 _striated
10 0 _smooth
9 1 _srand


TYPE STR# ;; Tools and performance controls
ToolsPerfControls,305
65
22 1 _chan
28 1 _vel
21 0 _velstep
0 0 _velcont
46 1 _script
47 1 _mod
43 0 _modstep
52 0 _modcont
58 1 _pitchbend
19 0 _pitchstep
54 0 _pitchcont
53 1 _press
55 0 _presstep
5 0 _presscont
7 2 _switchon
24 2 _switchoff
31 1 _volume
6 0 _volumestep
51 0 _volumecont
36 1 _legato
38 1 _staccato
41 0 _articulstep
39 0 _articulcont
40 0 _velfixed
37 0 _modfixed
8 0 _pitchfixed
10 0 _pressfixed
25 0 _volumefixed
29 0 _articulfixed
30 1 _pitchrange
9 1 _pitchrate
11 1 _modrate
13 1 _pressrate
26 1 _transpose
32 1 _volumerate
12 1 _volumecontrol
50 1 _pan
49 0 _panstep
59 0 _pancont
56 0 _panfixed
42 1 _panrate
48 1 _pancontrol
57 0 _rest
4 1 _ins
60 2 _value
20 1 _step
45 1 _cont
15 1 _fixed
14 0 _retro
61 0 _rndseq
33 0 _randomize
63 0 _ordseq
62 4 _keymap
64 0 _mapfixed
44 0 _mapcont
1 0 _mapstep
3 1 _rndvel
23 1 _rotate
2 2 _keyxpand
16 1 _rndtime
18 1 _srand
35 1 _tempo
27 0 _transposefixed
34 0 _transposecont
17 0 _transposestep

TYPE STR# ;; GeneralMIDI patches
GeneralMIDIpatches,306
128
1 AcousticGrandPiano 
2 BrightAcousticPiano
3 ElectricGrandPiano 
4 HonkyTonkPiano 
5 RhodesPiano
6 ChorusedPiano
7 Harpsichord
8 ClavinetChromatic 
9 Calesta
10 Glockenspiel
11 MusicBox
12 Vibraphone
13 Marimba
14 Xylophone 
15 TubularBells 
16 Dulcimer
17 DrawbarOrgan
18 PercussiveOrgan
19 RockOrgan
20 ChurchOrgan
21 ReedOrgan
22 Accoridan
23 Harmonica
24 TangoAccordian
25 AcousticGuitarNylon
26 AcousticGuitarSteel
27 ElectricGuitarJazz
28 ElectricGuitarClean
29 ElectricGuitarMuted
30 OverDrivenGuitar
31 DistortionGuitar
32 GuitarHarmonics
33 AcousticBass
34 ElectricBassFinger
35 ElectricBassPick
36 FretlessBass
37 SlapBass1
38 SlapBass2
39 SynthBass1
40 SynthBass2
41 Violin
42 Viola
43 Cello
44 Contrabass
45 TremoloStrings
46 PizzicatoStrings
47 OrchestralStrings
48 Timpani
49 StringEnsemble1
50 StringEnsemble2
51 SynthStrings1
52 SynthStrings2
53 ChoirAahs
54 VoiceOohs
55 SynthVoice
56 OrchestraHit
57 Trumpet
58 Trombone
59 Tuba
60 MutedTrumped
61 FrenchHorn
62 BrassSection
63 SynthBrass1
64 SynthBrass2
65 SopranoSax
66 AltoSax
67 TenorSax
68 BaritoneSax
69 Oboe
70 EnglishHorn
71 Bassoon
72 Clarinet
73 Piccolo
74 Flute
75 Recorder
76 PanFlute
77 BottleBlow
78 Shakuhachi
79 Whistle
80 Ocarina
81 Lead1 square
82 Lead2 sawtooth
83 Lead3 calliope
84 Lead4 chiff
85 Lead5 charang
86 Lead6 voice
87 Lead7 fifths
88 Lead8 bass+lead
89 Pad1 new age
90 Pad2 warm
91 Pad3 polysynth
92 Pad4 choir
93 Pad5 bowed
94 Pad6 metallic
95 Pad7 halo
96 Pad8 sweep
97 FX1 rain
98 FX2 soundtrack
99 FX3 crystal
100 FX4 atmosphere
101 FX5 brightness
102 FX6 goblins
103 FX7 echoes
104 FX8 sci-fi
105 Sitar
106 Banjo
107 Shamisen
108 Koto
109 Kalimba
110 Bagpipe
111 Fiddle
112 Shahnai
113 TinkleBell
114 Agogo
115 SteelDrums
116 WoodBlock
117 TaikoDrum
118 MelodicTom
119 SynthDrum
120 ReverseCymbal
121 GuitarFretNoise
122 BreathNoise
123 Seashore
124 BirdTweet
125 TelephoneRing
126 Helicopter
127 Applause
128 Gunshot

TYPE STR# ;; Performance controls
HTMLdiacritical,307
124
amp
&
gt
>
lt
<
quot
"
nbsp
 
AElig
Æ
Aacute
Á
Acirc
Â
Agrave
À
Aring
Å
Atilde
Ã
Auml
Ä
aelig
æ
aacute
á
acirc
â
agrave
à
aring
å
atilde
ã
auml
ä
Ccedil
Ç
ccedil
ç
Eacute
É
Ecirc
Ê
Egrave
È
Euml
Ë
eacute
é
ecirc
ê
egrave
è
euml
ë
Iacute
Í
Icirc
Î
Igrave
Ì
Iuml
Ï
iacute
í
icirc
î
igrave
ì
iuml
ï
Ntilde
Ñ
ntilde
ñ
Oacute
Ó
Ocirc
Ô
Ograve
Ò
Oslash
Ø
Otilde
Õ
Ouml
Ö
oacute
ó
ocirc
ô
ograve
ò
oslash
ø
otilde
õ
ouml
ö
Uacute
Ú
Ucirc
Û
Ugrave
Ù
Uuml
Ü
uacute
ú
ucirc
û
ugrave
ù
uuml
ü
Yacute
Y
yuml
ÿ
yacute
y
* ------------ Don't erase this last line ----------- *