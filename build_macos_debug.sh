#!/bin/bash
mkdir -p build && cd $_
cmake -G Xcode ..
xcodebuild -project PaintPrint.xcodeproj -target ALL_BUILD -config Debug

