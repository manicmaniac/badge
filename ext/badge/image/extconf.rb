require "mkmf"

exit unless have_framework "CoreFoundation"
exit unless have_framework "CoreGraphics"
exit unless have_framework "ImageIO"
create_makefile "badge/image/quartz"
