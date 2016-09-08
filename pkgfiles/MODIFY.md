% Modifying 'The Planet of the Vicious Creatures'
% Jorge Giner
% 28 August 2016

# Intro

The game is designed so you can easily modify or create new levels. This
document describes how you can start modifying the game.

Only the full version of the game can be modified.

# Unpacking the data

The game assets and levels come in a file named *data.pak*. This file is no
more than a *ZIP* file. To begin working with it, extract all the files it
contains into a floder named *data* in the same folder where *data.pak* is.
Now, rename *data.pak* to something else, for example *origdata.pak*.

Now, the game folder should contain something like this (on Windows):

~~~
planetvc.exe
origdata.pak
data/wasp.bmp
data/cosmonau.bmp
...
~~~

The game always tries to find the assets in the file *data.pak* before looking
for a directory named *data/*, because of that, we renamed *data.pak* to
*origdata.pak*.

# UTF-8

All the files, at least the files which  contain text strings with characters
not in the ASCII character set, should be saved in UTF-8 format.

# The contents

The game contents are:

- Some .bmp files which contain the graphics of the game.
- Some .wav files, the sounds of the game.
- Some .xm giles, the musics.
- Some r??.txt and i??.txt files, which describe each level of the game.
- Some bg\*.txt which describe common backgrounds for the levels.
- Some lang??.txt files, the translated text for each language.
- langlist.txt, the list of languages supported by the game.
- demo\_??.txt, saved gameplay for some levels.
- init.txt a very important file describing which level is loaded at each game
  state.

# The game states

The game can be in one of these states:

- *menu*: the main menu.
- *over*: we show the hi-score (with option to edit) and then game over.
- *end*: the player won the game.	
- *win*: after the end state, we go to a state similar to the over state, where
  you can put the hi-score, etc. But we can differenciate between this state
  and the over state. For example, right now the game shows a dead cosmonaut in
  the over state but not in this state.
- *gameplay*: playing.
- *demo*: watching how to beat the levels.

# The init file

This is a sample of the game init file:

~~~
start_map 0x02
active_portal 8
start_portal 1
translators_portal 2
menu_map 0x30
help_map 0x31
credits_map 0x33
over_map 0x40
end_map 0x41
win_map 0x43
demo 1
ndemos 49
nmaps_to_win 60
arrow 1
demo_version 0
~
~~~

- *start\_map* tells the level to load when the *gameplay* starts.
- *active\_portal* is the portal that should be active when the *gameplay*
  starts.
- *start\_portal* is the portal from where the cosmonaut exits when the
  *gameplay* starts.
- *menu\_map* is the level to load when the game enters the *menu* state.
- *help\_map* is the level to load when we select *HELP* on the menu.
- *credits\_map* is the level to load when we select *CREDITS* on the menu.
- *over\_map* is the level to load when the game enters the *over* state.
- *end\_map* is the level to load when the game enters the *end* state.
- *win\_map* is the level to load when the game enters the *win* state.
- *demo* is 1 if *DEMO* in the menu does something.
- *ndemos*, the number of demo\_??.txt files that we have.
- *nmaps\_to\_win*, the number of levels we need to pass to win the game.
- *demo\_version* if the game is a demo or not (should be 0).

The init file must end with a tilde `~` on alone on a line.

# The levels

Each level is described by a *room* file, for example *r0f.txt*, and an
optional *init* file, which must have the same name except that it starts with
an '`i`', that is, in this case *i0f.txt*

This *0f* is an hexadecimal number. Hexadecimal numbers are composed by these
digits '0123456789abcdef'. As you can have 2 digits, this means that the
maximum posible number of combinations is 256, so you can have at maximum 256
levels in the game.

You can imagine the levels as forming a 16 by 16 grid:

~~~
00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
...
e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb ec ed ee ef
f0 f1 f2 f3 f4 f5 f6 f7 f8 f9 fa fb fc fd fe ff
~~~

When the cosmonaut exits from one level to the right, the level at the right on
this grid is loaded, or the level at the left side if she exits to the left.
For example, if we exit from level 06 to the right, level 07 will be loaded,
and level 05 will be loaded if we exit to the left.

Moreover, each row forms a ring: that means that if you exit level 00 to the
left, the level 0f will be loaded; and if you exit level 0f to the right, level
00 will be loaded. This implies that the cosmonaut can only travel 16 levels at
maximum: to use more, you have to place portals to travel to another level on
another row of the above grid.

## Modifying while playing

The best way to work is to go to the level you want to modify in the game. Open
the *room* and *init* file for the level with a text editor and start
modifying. Then, in the game, simply restart the room (pressing the `spacebar`
by default): the room will be reloaded from disk and you will see your changes
immediatlely.

There is not a way to reach to the level that you want to modify immediately, but you can try 2 things:

- Specify in *init.txt* the *start_map* as the level you want to modify so it
  loads when you just enter gameplay.
- While at the menu, write '`abusimbel1`' . A number `1` should appear at the
  up right corner of the screen. Then, when you enter the game, you can use the
  keys 1, 2, 3, 4 to put the cosmonaut on one of the four virtual positions you
  defined with the `spawn cosmonaut` command; and you can use as well the key
  `O` (letter o) to give you oxygen immedately. This way you can go quickly to
  the level that you want to modify.

## The room file

Each room file `r??.txt` specifies the background and tiles of the level. For
example:

~~~
bg10.txt
21 22 23 20 22 21 20 23 20 23 21 22 23 21 22 20
22 82 83 00 26 00 00 27 00 00 26 00 27 26 00 27
21 92 00 00 00 00 00 00 00 00 00 00 00 00 00 00
23 20 1F 00 00 00 00 00 00 00 00 00 00 00 00 00
20 21 1F 00 00 00 00 00 00 00 00 00 00 00 00 00
22 23 22 1F 24 00 00 25 24 00 00 00 00 00 00 00
20 21 23 21 22 20 21 22 21 23 21 1F 00 25 24 00
21 1F 26 00 27 1E 22 23 20 21 22 23 20 21 22 23
23 1F 00 00 00 00 26 1E 21 20 23 20 22 1F 27 00
22 1F 00 00 00 00 00 00 1E 22 21 1F 00 00 00 00
21 1F 00 00 00 00 00 00 00 00 26 00 00 00 00 00
22 21 1F 25 24 00 00 25 00 24 00 00 00 25 24 00
20 22 23 21 22 20 23 20 21 22 21 23 22 20 21 23
FE FE FE FE FE FE FE FE FE FE FE FE FE FE FE FE
~~~

The first line is the name of a background file to load for the background
graphics of the room. If we don't want a background, write a tilde `~` alone on
that line.

After that, there are 14 rows of 16 hexadecimal numbers. Each number
corresponds to one tile of the file *tileset.bmp*. This bitmap file is arranged
to have 16 by 16 tiles, so there is a direct mapping between the number we
write here and the tile we choose. For example, if we use 3A, the tile on row 4
column 11 (A is the decimal number 10) will be selected (we count from 0).

## The background file

The background file specifies some graphics to put at some positions to make
the background of the room. For example, this is *bgplanet.txt*:

~~~
planet 12 2
star1 1 5
star1 2 9
star1 3 2
star1 4 8
star1 6 4
star1 9 1
star1 9 6
star1 11 10
star1 13 8
.
~~~

Each line is formed with the id of the graphic to draw and the position in tile
coordinates. Each graphic is drawn in order to make the background, so the
later can draw over the former.

The background file *must* end with a dot '`.`'.

The ids for the graphics and their dimensions are hardcoded in the game. They
are:

- *planet* to draw the rect at position (0,0) and dimensions (48, 48) from
  *blocks.bmp*.
- *star1* to draw the rect at position (48, 0) and dimensions (16, 16) from
  *blocks.bmp*.
- *stargate* to draw the rect at position (0, 0) and dimensions (96, 96) from
  *stargate.bmp*.
- *endscr* to draw the rect at position (0, 0) and dimensions (256, 224) from
  *end.bmp*.

## The init file

The init file is optional. If specified, it contains a list of commands to
execute when the level is loaded. For example:

~~~
exits 1 1 0 0 ~
spawn grasshopper 12 4 SLRSLRUULR ~
spawn grasshopper 5 11 SUSRLSUUURL ~
spawn grasshopper 8 11 LSRSULRSUUS ~
spawn oxigen 7 8 ~
spawn oxigen 3 2 ~
spawn cosmonaut 0 11 14 11 0 5 14 4 0 ~
.
~~~

As you see, each line starts with a command, continues with a variable number
of arguments to the command, and ends with a tilde `~`. It is *important* to
separate this tilde at least with one space from the previous word. The only
exception is when the last word is a text string (for example for the *text*
commands). In this case don't let any space between the text string and the
tilde, or you will be adding a space to the end text.

The last line *must* contain a dot `.` .

Let's see all the commands available one by one.

### exits command

    exits <left> <right> <up> <down>
    example: exits 1 1 0 0

The *exits* command needs 4 numbers that must be 0 or 1. Each one tells the
game if the player can exit from the current level from the left, right, top or
bottom in this order. A number 1 means that there is an exit to that side. Top
and bottom, that is, the last two, must be always 0. 

### spawn command

    spawn <what> <x tile pos> <y tile pos> [arguments]

The *spanw* command is used to populate the room with enemies or other objects. We always specify the id of the object to spawn and its initial position. Then, each object can take different arguments.

#### bat

    spawn bat <x> <y> <dir>
    example: spawn bat 7 8 ldir

Spawns a bat, `<dir>` can be `ldir` to start looking to the left, or `rdir` to
start looking to the right.

#### caterpillar

    spawn caterpillar <x> <t> <flip>
    example: spawn caterpillar 7 8 none

Spawns a caterpillar. `<flip>`can be `flipv` to draw the caterpillar upside
down (to place it on ceilings), or anything else to draw it on the ground.

#### cosmonaut

    spawn cosmonaut <x> <y> <x0> <y0> <x1> <y1> <x2> <y2> <x3> <y3> <def>
    example: spawn cosmonaut 10 11 0 10 13 10 0 13 7 1 7 0 ~

Spawns the cosmonaut that can be controlled by the player. The `<x0>`, `<y0>`,
etc. are tile coordinates that define 4 virtual start positions for the
cosmonaut in the level, and `<def>` is a number between 0 and 3 specifying
which of these four positions is the default. This is used for 2 things: for
testing the level and to define the positions of the helper arrow that shows
when you take an oxygen bubble.

The virtual positions should be in this order left-bottom, right-bottom,
left-top, right-top, because the game always makes the helper arrow to point to
the left for the positions 0 and 2, and to the right for the positions 1 and 3.

#### deadcosmo

    spawn deadcosmo <x> <y>

Spawns a dead cosmonaut.

#### draco

    spawn drawco <x> <y> <dir> <action string>
    example: spawn draco 7 8 rdir SSASB

Spawns a dragon. `<dir>` can be `ldir` to look to the left or `rdir` to look to
the right.

The `<action_string>` is a maximum of 15 consecutive upper case letters that
can be: S, A, B or C. Each letter specifies an action for the dragon to do. The
dragon does the first action specified and, when the action finishes, executes
the next. When the last action is executed it starts from the beginning. The
actions are:

- *S*: stay idle for 2 seconds.
- *A*: spit fire during 2 seconds.
- *B*: spit fire during 3 seconds.
- *C*: spit fire during 4 seconds.

#### drop

    spawn drop <x> <y> <accel> <color>
    example: spawn drop 7 1 16 1

Spawns a drop that will fall until it reaches the ground or a platform.
`<accel>` is the acceleration. `<color>` can be 0 or 1.

#### fenix

    spawn fenix <x> <y> <dir>

Spawn a fenix bird. The syntax is the same as for the bat.

#### fish

    spawn fish <x> <y> <dir> <frames> <vy> <ay>
    example: spawn fish 5 11 rdir 48 -850 16

Spawn a fish. `<dir>`can be `ldir` or `rdir` to make it look left or right.
`<frames>` is the number of frames to wait until shooting (30 is one second).
`<vy>` is the initial speed of the shot and `<ay>` the gravity.

#### grasshopper

    spawn grassshopper <x> <y> <action_string>
    example: spawn grasshopper 2 11 RSURSLLRSUL

Spawns a grasshopper. As for the dragon, the action string is a series of upper
case letters. In this case they are:

- *S*: stay idle for 3 seconds.
- *U*: jump up.
- *L*: jump left.
- *R*: jump right.

#### lavashot

    spawn lavashot <x> <y> <two_tiles> <frames> <vy> <ay>
    example: spawn lavashot 4 12 0 10 -800 16

Spawns a lava rock. By default it is placed in the middle horizontal position
of the tile `<x>`, but specifying `<two_tiles>` as 1, it will be placed between
that tile and the next. `<frames>` is the number of frames to wait before
jumping (30 is one second). `<vy>` is the initial velocity and `<ay>` the
gravity.

#### lavatongue

    spawn lavatongue <x> <y> <action_string>

Spawns a lava tongue. The `<action_string>`is formed from this set:

- *S*: don't show for one second.
- *L*: show, going to the left.
- *R*: show, going to the right.

#### moth

    spawn moth <x> <y>

Spawns an electric moth.

#### oxigen

    spawn oxigen <x> <y>

Spawns an oxygen bubble. Sorry for the typo, but in the game you must use
`oxigen` instead of `oxygen`.

#### spider

    spawn spider <x> <y> <radius> <aw> <start_angle>
    example: spawn spider 3 1 34 64 45 

Spawns a spider. The spider will hang from position `<x>`, `<y>`. The
`<radius>` is the length of the thread in pixels. `<aw>` is the angular
acceleration and `<start_angle>` is the angle in degrees where to start.
(*TODO*: check this.)

#### wasp

    spawn wasp <x> <y> <dir>
    example: spawn wasp 5 7 ldir

Spawns a wasp. `<dir>` can be `ldir` or `rdir`.

#### platform

    spawn platform <x> <y> <anim> <anim_speed>
    example: spawn platform 0 0 0 4

Spawns a platform. `<anim>` can be:

- 0 for the blue-stars platform.
- 1 for the yellow acid cloud platform.
- 2 for the violet acid cloud platform.

`<anim_speed>` is the animation speed: it is the number of frames to show the
current animation frame before passing to the next one.

#### gameover

    spawn gameover <x> <y>

Spawns the GAME OVER animated text. Only the `<y>` coordinate is used: it is
centered horizontally.

#### menu

    spawn menu <x> <y> <text>
    example: spawn menu 0 0 JOHANNA BRAHE|IN|© 2016 JORGE GINER~ 

Spawns the menu and draws some header and footer text. '<text>' must be a
string with three elements separated by a `|` . The two first are drawn at the
top usgin two lines. The last is drawn at the bottom after the version number.


#### hiscore

    spawn hiscore <x> <y>

Spawns the hi-score table.

#### stargate

    spawn stargate <x> <y> <id> <gotorid> <gotogateid> <nextgate> <ac> <arrowd>
    example: spawn stargate 7 5 8 0x18 3 4 0 2

Spawns a stargate.

- `<id>` will be the id number of this stargate.
- `<gotorid>` is the level number we will go to, if we enter this stargate.
- `<gotogateid>`: when we load `<gotorid>`, exit through the stargate with this
  id.
- `<nextgate>`: id of the next gate that should be active when we go through
  this stargate.
- `<ac>` if the gate should start in active state (showing plasma). Not used,
  should be 0. The inital active portal is selected using *init.txt* .
- `<arrowd>`: a number between 0 and 3. When we go through this gate, sets the
  direction at which the on screen helper arrow should point for the following
  levels. It selects one of the virtual positions that we specify when we spawn
  the cosmonaut.

### music

    music <music_file_name>
    example: music lava.xm

Starts a music file.

### new\_path

    new_path <tx> <ty>

Starts a new path. The first position for the path, in tile coordinates, will
be `<tx>`, `<ty>`. The next `add_path_point` commands will add points to this
path. The first `new_path` command that we see in the level init file will be
the path number 0, the next the number 1, etc. We will use that id later to
assign paths to spawned objects.

### add\_path\_point

    add_path_point <tx> <ty>

Adds a point to the last path created with `new_path`.

### set\_path

    set_path <id> <pointnum> <speed> <loop>
    example: set_path 2 0 96 1

Gets the last spawned actor and sets a path to follow. `<id>` is the number of
the path. `<pointnum>` is the starting point number in the path (from 0).
`<speed>` is the speed at which the actor will move on the path. `<loop>` is 1
or 0. 1 means that when the actor reaches the end of the path it will came
back.

### wait

	wait

Waits for the player to press `RETURN` or `ESC`.

### wait\_hiscore

    wait_hiscore

When we spawn *hiscore* it can happen that we can have to enter our initials or
not.  This waits until we finish entering our initials or does nothing
otherwise.

### text

    text <x> <y> <string>
    example: text 1 7 PORTALS, FOUND BURIED...

Draws a text waiting a little at each character drawn. The text must be upper
case. `<x>`, `<y>` is the tile postion where we start drawing, but this tile
positions are in 8x8 pixels coordinates (instead of 16x16). Does automatic text
wrapping.

### textf

    textf <x> <y> <string>
    example: text 1 7 PORTALS, FOUND BURIED...

Like *text* but draws all the characters immediately.

### textfc

    textfc <x> <y> <w> <string>
    example: textc 1 7 24 PORTALS, FOUND BURIED...

Like *textf* but draws all the characters centered. It will center the string
at `x + w / 2`.

### clrscr

    clrscr

Clears all text drawn.

### scroll

    scroll <level id>
    example: scroll 0x30

Makes the screen to scroll from the current loaded level to the level
specified, and then execute the new level init script.

### load

    load <level_id>
    example: load 0x30

Loads a level immediately. The next script commands are not executed.

### endstate

    endstate

Ends the current state and goes to other state. The next commands are not
executed. Each state specifies where to go when we call *endstate*:

- *menu*: exits the game.
- *over*: goes to the *menu* state.
- *end*: goes to the *win* state.
- *win*: goes to the *menu* state.
- *gameplay*: goes to the *menu* state.
- *demo*: goes to the *menu* state.

### fadeout

    fadeout

Fades to black.

### fadein

    fadein

Fades from black.

# Languages

The file *langlist.txt* contains the languages that the game supports.

~~~
ESPAÑOL
es
FRANÇAIS
fr
ITALIANO
it
LSF (PEANO)
il
POLSKI
pl
PORTUGUÊS
pt
~
~~~

We have first the name of the language to show on the menu, then the language
identifier. This file must end with a tilde `~`.

To search for the language translations, we look for a file named *lang??.txt*,
where *??* is substituted by the 2-letter language id.

Each language file is made by pair of lines. The first is always the line in
English, the second is the same line but translated. The file *must* end with a
tilde `~`.

The lines can have a escape code formed by two characters: `\n`. This is used
to instert a line break.

# Demo files

To be described.

