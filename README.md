Precimonious
===========

Precimonious employs a dynamic program analysis technique to find a lower
floating-point precision that can be used in any part of a program.
Precimonious performs a search on the program variables trying to lower their
precision subject to accuracy constraints and performance goals. The tool then
recommends a type instantiation for these variables using less precision while
producing an accurate enough answer without causing exceptions.
