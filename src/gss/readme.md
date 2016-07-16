GSS: GruesomeSpaceScript

It's a pretty simple script engine. I wrote my own instead of using something off the shelve because:
* I wanted to do it.
* Full control and understanding.
* Many script engines are difficult to serialize.
* Tight memory management and error control.

It is a simple stack based runtime engine, with a pre-compile step into GSS specific instructions.
It does variable name checks at compile time, instead of most script engines doing these checks at runtime. This makes it a bit safer at runtime. However, it does require all native bindings to be registers pre-compile time.

It is incomplete. It has partial support for lists, no support for dictionaries (the type in GssVariant is a placeholder)

It has a pretty basic garbage collector (GC), which just copies all used data to a new memory block when GC needs to happen.