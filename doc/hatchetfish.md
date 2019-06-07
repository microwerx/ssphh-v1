# Hatchetfish Guidelines

- `HFLOG(...)` prints a normal colored log message
- `HFLOGINFO(...)` prints a cyan colored log message
- `HFLOGWARNING(...)` prints a yellow colored log message
- `HFLOGERROR(...)` prints a red colored log message
- `HFLOGDEBUG(...)` prints a magenta colored log message

These messages can be toggled on and off and saved to an alternate file stream.

- `HFLOG` print normal messages that should not be seen on screen.
- `HFLOGINFO` print messages for main section messages
- `HFLOGDEBUG` print detailed messages about internal activity
- `HFLOGWARNING` print messages about noncritical activity
- `HFLOGERROR` print messages about critical activity