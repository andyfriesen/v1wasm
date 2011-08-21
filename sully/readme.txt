VERGE readme file (04/Jun/98)
=============================
Written by Ric (riclau@mailcity.com)

Zeromus is rather busy at the moment so has asked Aen (Fist1000@aol.com),
NichG (nickh@erols.com) and I to put together this release between us.

Okay so here it is, the new version of VERGE. We've added quite a lot of
things to the engine in this release, mainly extending VergeC. Please read
this file (and the other docs) because useful information is contained
within.

Due to a mix up there's no pack-in demo included with this release. Instead
this zip contains the necessary updated dat files and binary exes that are
required for this version that are not in the last verge.zip. It is highly
recommended that you download that if you already haven't from the VERGE
homepage (http://moria.vivid.com/vecna/).

For information on VERGE, consult VERGEDOC.TXT, VERGEC.TXT and VERGE.FAQ,
which should be included with this file.

What's new in this release (04/Jun/98)
--------------------------------------
- The VC layer functions now work in startup.vc.
- More VC functions, including:
    Magic functions
    Menu functions
    CallScript and CallEvent to call scripts in STARTUP.VC and EFFECTS.VC
    BindKey for calling vc scripts on keypresses
    Built in VAS support for animation playback
- Dynamic memory allocation. VERGE now allocates only as much memory as
  required for the maps and vsps. The only buffer that you need to change
  manually in setup.cfg is vcbuf - and that's only if you intend to use
  more than the default allocated amount (eg if you wanted to use 
  the new PlayVAS).
- Error checking code added to the memory allocation routines.
- Tarkuss/southern border bug fixed
- Intensity calculation routines (for the fades) corrected.

Misc
----
- A previously undocumented feature of VERGE is the built in facility
  to take screenshots which is activated by pressing <F10>.

Important
---------
Okay here's some pointers on how to make your existing files work with this
version.

1) The character dat files have changed slightly and require an extra column
   for magic gained at a certain skill level (see magic under VERGEDOC.TXT).
2) The new files required for magic: magicon.dat, magic.dat, magic.vcs and
   magiceq.dat must be included for main.exe to run.
3) There's a problem with all the previous versions of NEWMAP.EXE (not the 
   new one created by aen :)). It will incorrectly write one byte of 
   information as the entity chr filename for entity 5 into your map. If 
   you've created a map with NEWMAP and find that it pagefaults please 
   check that there isn't some weird character for the fifth entity's chr
   filename.
4) And on that note, please make sure that any entity chr filenames specified
   exist. Your map will crash if VERGE cannot find one of the specified entity
   chrs.
5) EnforceAnimation has been disabled in this release. It's still there but
   doesn't actually do anything. The reason for this is that we have not been
   able to recreate the bug which this command was introduced for. If you do
   come across problems because of this, please email one of us and we'll
   reenable it immediately.

Credits
-------
VERGE in it's current form wouldn't exist without:
  vecna (vecna@inlink.com)     - original creator of VERGE
  hahn (hahn@ll.net)           - author of the pack-in demo
  zeromus (zeromus@flash.net)  - current programming leader of VERGE
  Locke (locke@vivid.com)      - for maintaining the VERGE repository and
                                 discussion board
  McGrue (McGrue@juno.com)     - maintainer of the VERGE FAQ
  xBig_D (xBig_D@mailcity.com) - creator of the original VAS format,
                                 implemented the second VC layer and the
                                 modposition variable
  Aen (Fist1000@aol.com)       - editor of the docs in this release,
                                 helped with bugfixes in this release and
                                 wrote the dynamic memory routines
  NichG (nickh@erols.com)      - implemented the magic system and other new
                                 functions in this release
  Ric (riclau@mailcity.com)    - programming and bugfixing in this release

And finally everyone else out there in the VERGE community for your support
and demos. 

- Ric (riclau@mailcity.com)


