# Tiler

Windows DLL to layout windows by name prefix.
Layout configuration rules are in ```%AppData%/tiling``` file.
Mode is either grid (cols, rows specification required) or explicit (directly place windows according to rect coordinates). Process specification required.

### Format

```
[WindowTitlePrefix]
mode = grid|explicit
cols = <number>
rows = <number> | auto
rects = x,y,w,h; x,y,w,h; ...
process = process_name.exe
bring_to_top = true|false
```

### Example

```
[Note]
mode = grid
cols = 2
rows = auto
process = notepad.exe
bring_to_top = true

[Calc]
mode = explicit
rects = 0,0,400,300; 400,0,400,300
process = calculator.exe
bring_to_top = false
```




