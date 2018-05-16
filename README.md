PaintPrint
==========

Convert a digital image to gcode for painting thanks to a 3D printer.

I initially designed this tool for watercolor sepia transfers, with multiple passes.

FEATURES
--------

-   Paintbrush width, color, drag error compensation

-   Paint refill management with custom gcode

-   Multiple-pass (layers) and layer dry time

-   Preview of the strokes per layer and preview of the blended output

EXAMPLE
-------

~~~~
PaintPrint -i resources/dogs-2921382-640.jpg \
           -o resources/dogs-2921382-640.output \
           -ow 80 \
           -pa 200 200 \
           -tr 1.5 "#ddddfa" 2.5 resources/toolrefill.paintbrush.gcode 300 20 \
           -l 0.25 \
           -l 0.45 \
           -l 0.65 \
           -l 0.85
~~~~
The first two arguments are input and output, then the width in mm of the printed image and the print area dimensions in mm, then the definition of a tool that needs to periodically refill, its width, color, drag error, its refill command, the length it can paint in mm before refilling and the drying wait time in seconds, and four layers with different luminosity threshold.

Here are the input and output previews of this command.

`TODO`

Here is a print of the generated gcode file.

`TODO`

FAQ
---

-   What are the maximum dimensions of the input image? A maximum of about 1280 pixels width or height is reasonable

-   This is slow?! Please rather use a Release build with optimizations. Once multithreading will be implemented, it should be even faster.

TODO
----

-   A better WYSIWYG preview

-   Example print

-   Verbose mode

-   Multithreading

-   Paint fill direction setting for every layer

-   Better isolated dots or holes deletion

-   Tool configuration via a json file

-   Global project configuration via a json file

-   Multiple tools/colors

-   Maybe embroidery transfers options
