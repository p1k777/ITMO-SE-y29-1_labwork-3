# Laboratory Work 3. ITMO y29 Software Development

**This repository is a copy of the private repository of the first
laboratory work of the first year at ITMO, program -- Software
Development**

## Purpose of the work

Write a command-line argument parser.

## Working with the library

Into the ~~class~~ structure `ArgumentParser`, *positional* and *named*
arguments and flags must be added through the corresponding functions.
Further interaction will be performed through functions such as
`nargparse::GetRepeated` and others.

## Project structure

-   `Container` --- ~~class~~ structure analogous to a vector (with
    reduced functionality and completely non‑STL-compatible),
    interaction via the functions below
-   `ContRealloc` --- reallocates memory for `Container`
-   `ContPushBack` --- adds an element to the end of `Container`
-   `ArgParser` --- the main parser structure with the required fields
-   `ArgsN` --- enum specifying the number of arguments of a certain
    type/flag
-   `Flag` --- flag structure
-   `ValueType` --- enum specifying the argument type
-   `Validator` --- validator for an argument of an arbitrary type
    (therefore it uses `union`)
-   `PosArg` --- positional argument structure
-   `NamedArgs` --- named argument structure
-   `AddFlag` --- adds a flag to the parser
-   `AddArgument` --- adds an argument (positional or named depending on
    parameters) to `ArgParser`
-   `Parse` --- calls all required functions to parse a set of arguments
-   `GetRepeatedCount` --- if an argument can be obtained multiple
    times, shows how many were received
-   `GetRepeated` --- copies the value of an argument with a specified
    index into `target`
-   `AddHelp` and `PrintHelp` --- work with the help block
-   `FreeParser` --- frees all memory allocated for the parser

## More details

Use **cmake** for building.

There are tests in the repository. To run the tests, use **cmake**.

See `task_README.md`.

## P.S.

The work is implemented in a hacky way and may work incorrectly. The
reason is the author's inexperience and some artificial constraints set
by the instructor (see the same `task_README.md`).
