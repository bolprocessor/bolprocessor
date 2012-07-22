	sr = 22050
	kr = 2205
	ksmps = 10
	nchnls = 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   instrument 1   "Vina"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	p1	=	1
;	p2	=	start time
;	p3	=	duration
;	p4	=	pitch in octave.pitchclass notation
;	p5	=	volume start value
;	p6	=	volume end value
;	p7	=	volume function table number
;	p8	=	continuous PB start value
;	p9	=	continuoue PB end value
;	p10	=	continuoue PB function table number
;	p11	=	discontinuout PB start value
;	p12	=	discontinuout PB end value
;	p13	=	discontinuous PB function table number
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; If any of the function table numbers are negative, 
; that corresponding parameter will be interpolated linearly.
; (Bezier curves later!)
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	instr	1
;	ihold
;	Scaling factors
ivolscale	= 32767.0 / 127.0
ipitchscale	= log(2.0) / 1200.0

;	sound duration
idur		=	p3
icpspitch		=	cpspch(p4)

;	set function table numbers
ifdiscontpb	=	p13
ifcontpb	=	p10
ifvol		=	p7

;	set continuous parameters
ivol1		=	p5
ivol2		=	p6
icontpb1	=	p8
icontpb2	=	p9
idiscontpb1	=	p11
idiscontpb2	=	p12


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; volume block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

if (ifvol <= 0) goto linearvolume

ivollength	=	ftlen(ifvol)
kvolindex	line	0,	idur,	ivollength
kvol		tablei	kvolindex,	ifvol
goto continuea

linearvolume:	kvol line ivol1, idur, ivol2


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; continuous pitch bend block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

continuea:	if (ifcontpb <= 0)	goto linearcontpb

icontpblength	=	ftlen(ifcontpb)
kcontpbindex	line	0,	idur,	icontpblength
kcontpb			tablei	kcontpbindex,	ifcontpb
goto continueb

linearcontpb:	kcontpb line icontpb1, idur, icontpb2


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; discontinuous pitch bend block
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

continueb:	if (ifdiscontpb <= 0) goto lineardiscontpb

idiscontpblength	=	ftlen(ifdiscontpb)
kdiscontpbindex		line	0,	idur,	idiscontpblength
kdiscontpb			tablei	kdiscontpbindex,	ifdiscontpb
goto continuec

lineardiscontpb:	kdiscontpb line idiscontpb1, idur, idiscontpb2

continuec: kdiscontpb	=	int(kdiscontpb / 100.0) * 100.0


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Main section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;ioctpitch	=	octpch(p4)
;icpspitch	=	cpspch(p4)
;			print	p4, ioctpitch, icpspitch
;koctpitch	=	ioctpitch + ((kcontpb + kdiscontpb) / 1200.0)

kcpspitch	=	icpspitch * exp(ipitchscale * (kcontpb + kdiscontpb))
kamp		=	kvol * ivolscale

		display	kcontpb, 2
		display	kdiscontpb, 2
		display kcpspitch, 2
		display kvol, 2
asignal	pluck kamp,	kcpspitch, icpspitch,	1,	5, 0.916, 0.083

	out	asignal * 0.5
	
	endin




