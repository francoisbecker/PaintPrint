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
