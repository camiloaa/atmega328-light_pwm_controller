Creating my own light controller with a ATmega328p

The code is a mess so far.

I'm not sure yet if it's a good idea to allow direct writing of the DC_OUT register.

On one hand it give the chance to do anything from the controlling host.

On the other, it adds an extra step (writing to STEP register) before changing duty cycles,
and requires very good understanding of what you're doing.

