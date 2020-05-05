#!/bin/sh
# uses imagemagick convert to generate all the required resolutions from a nice big square design (1024x1024 works well)
# note that 'mogrify -strip image.png' can be used to get rid of some of the load errors in Qt
# note online converters like https://icoconvert.com/ may do a better job
# or use the javascript resize_for_icon.jsx file in adobe photoshop

# convert 'dino_cpu (5300x5300).png' -bordercolor white -border 0 \( -clone 0 -resize 16x16 \) \( -clone 0 -resize 24x24 \) \( -clone 0 -resize 32x32 \) \( -clone 0 -resize 40x40 \) \( -clone 0 -resize 48x48 \) \( -clone 0 -resize 64x64 \) \( -clone 0 -resize 256x256 \) -delete 0 -alpha off -colors 256 icon_256x256.ico


# convert resized_icons/icon1024x1024.png resized_icons/icon128x128.png resized_icons/icon16x16.png resized_icons/icon192x192.png resized_icons/icon24x24.png resized_icons/icon256x256.png resized_icons/icon32x32.png resized_icons/icon40x40.png resized_icons/icon48x48.png resized_icons/icon512x512.png resized_icons/icon64x64.png resized_icons/icon96x96.png icon_256x256.ico

convert resized_icons/icon128x128.png resized_icons/icon16x16.png resized_icons/icon192x192.png resized_icons/icon24x24.png resized_icons/icon256x256.png resized_icons/icon32x32.png resized_icons/icon40x40.png resized_icons/icon48x48.png resized_icons/icon64x64.png resized_icons/icon96x96.png icon_256x256.ico

cp resized_icons/icon256x256.png Icon.iconset/icon_128x128@2x.png
cp resized_icons/icon128x128.png Icon.iconset/icon_128x128.png
cp resized_icons/icon32x32.png Icon.iconset/icon_16x16@2x.png
cp resized_icons/icon16x16.png Icon.iconset/icon_16x16.png
cp resized_icons/icon512x512.png Icon.iconset/icon_256x256@2x.png
cp resized_icons/icon256x256.png Icon.iconset/icon_256x256.png
cp resized_icons/icon64x64.png Icon.iconset/icon_32x32@2x.png
cp resized_icons/icon32x32.png Icon.iconset/icon_32x32.png
cp resized_icons/icon512x512.png Icon.iconset/icon_512x512@2x.png
cp resized_icons/icon1024x1024.png Icon.iconset/icon_512x512.png
