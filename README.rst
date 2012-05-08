Clockie
=======

Clockie is a really simple clock I made for Ann-Marie (my girlfriend) when she
moved away for her master's degree. It is based on an Arduino and a simple LCD
character display and features a countdown until we next meet (d'awwwww).  The
time (and the date of our next meeting) is set by holding it up to a web page
which literally flashes the time at a light sensor on the back of the clock.

Video: http://www.youtube.com/watch?v=T5hS6n_OzEQ

.. figure:: https://raw.github.com/mossblaser/Clockie/master/images/moneyshot.jpg
	
	Clockie being set (left), showing the countdown (top right) and its
	presentation gift-box (bottom right).

Why This Design?
----------------

When I designed Clockie I was temporarily living in Sheffield for my summer job
at ARM and so only had a handful of components available. In particular, I
didn't have a real-time clock or anything for building an efficient
battery-powered device. As a result Clockie forgets the time every time its
unplugged and must be set again. Without any buttons to hand and
motivated further by my dislike of every button-based digital clock setting
interface I've ever used, I decided on using a computer to set the time.

Unfortunately, Ann-Marie is a Windows user and I didn't fancy writing and
testing a piece of software for communicating with the clock over USB for a
platform I don't use. Also, who wants to find a USB cable lying about just to
set your clock? Inspired by the blinding glow of opening a white terminal window
in a dark room late at night I realised I could use a web page and a light
sensor to interface with the computer.

Building Hardware
-----------------

Clockie is made out of:

* An Arduino I had lying around (funnily enough this board was very first
  microcontroller I ever bought).
* A super-cheap green HD44780-compatible LCD display off ebay (because
  Ann-Marie's favourite colour is green...).
* An LDR (light-dependant resistor, not long distance relationship...)
* A random plastic case I had lying around

The case, and the dental drill used to cut it up, was stolen from my dad (a
dentist) and finally stuck together using that most premium of materials:
white-tack. The Arduino and LDR were simply glued into the box and the LCD
screwed into the lid.

A `potential-divider <http://en.wikipedia.org/wiki/Voltage_divider>` circuit
which allows the Arduino to measure the LDR's resistance and thus the amount of
light. This was precariously soldered together between the legs of the LDR which
appeared to work reasonably well despite the awful wire I had lying around.

.. image:: https://raw.github.com/mossblaser/Clockie/master/images/case.jpg

Not the most polished piece of hardware ever made but certainly "good enough".
If that level of effort doesn't say "I love you" I don't know what will ;).

Communication
-------------

The clock requires the current time and the time of our next meeting to be
transmitted. These take the form of a pair of 32-bit UNIX timestamps (the number
of seconds since 1970). To transmit the data I used `Manchester encoding
<http://en.wikipedia.org/wiki/Manchester_code>`. I could have used something
similar to Morse code but seeing as we were both at the University of Manchester
where it was invented it seemed rather fitting even though it is rather more
complicated and doesn't really offer any advantages in this case.

Video of the prototype system: http://www.youtube.com/watch?v=1osQdIe3dp8

The prototype worked rather well on my computer late the night I built it but
unfortunately didn't work so reliably on other random computers I tried.  To
make things work reliably on slow computers and slow-to-update screens I slowed
everything down to a blistering 5 bits per second meaning that setting the time
takes just under 13 seconds. Sadly this wrote off any possibility of sending
messages as well as the time as they would take far too long to send.


