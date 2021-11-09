fitrack_icon := assets/icon175x175_fitrack.png
testflight_icon := assets/icon175x175.png

.PHONY: create-assets build test

create-assets:
	@echo "--> Recreating all assets for readme"
	cp $(fitrack_icon) assets/icon175x175_fitrack_light_badged.png
	bin/badge --glob "/assets/icon175x175_fitrack_light_badged.png"
	cp $(testflight_icon) assets/icon175x175_alpha_light_badged.png
	bin/badge --glob "/assets/icon175x175_alpha_light_badged.png"
	cp $(testflight_icon) assets/icon175x175_light_badged.png
	bin/badge --glob "/assets/icon175x175_light_badged.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_dark_badged.png
	bin/badge --dark --glob "/assets/icon175x175_fitrack_dark_badged.png"
	cp $(testflight_icon) assets/icon175x175_dark_badged.png
	bin/badge --dark --glob "/assets/icon175x175_dark_badged.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_alpha_light_badged.png
	bin/badge --alpha --glob "/assets/icon175x175_fitrack_alpha_light_badged.png"
	cp $(testflight_icon) assets/icon175x175_alpha_light_badged.png
	bin/badge --alpha --glob "/assets/icon175x175_alpha_light_badged.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_shield_1.2-2031-orange.png
	bin/badge --shield "1.2-2031-orange" --no_badge --glob "/assets/icon175x175_fitrack_shield_1.2-2031-orange.png"
	cp $(testflight_icon) assets/icon175x175_shield_1.2-2031-orange.png
	bin/badge --shield "1.2-2031-orange" --no_badge --glob "/assets/icon175x175_shield_1.2-2031-orange.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_shield_1.2-2031-orange-no-resize.png
	bin/badge --shield "1.2-2031-orange" --no_badge --shield_no_resize --glob "/assets/icon175x175_fitrack_shield_1.2-2031-orange-no-resize.png"
	cp $(testflight_icon) assets/icon175x175_shield_1.2-2031-orange-no-resize.png
	bin/badge --shield "1.2-2031-orange" --no_badge --shield_no_resize --glob "/assets/icon175x175_shield_1.2-2031-orange-no-resize.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_shield_Version-0.0.3-blue.png
	bin/badge --shield "Version-0.0.3-blue" --dark --glob "/assets/icon175x175_fitrack_shield_Version-0.0.3-blue.png"
	cp $(testflight_icon) assets/icon175x175_shield_Version-0.0.3-blue.png
	bin/badge --shield "Version-0.0.3-blue" --dark --glob "/assets/icon175x175_shield_Version-0.0.3-blue.png"

	cp $(fitrack_icon) assets/icon175x175_fitrack_shield_Version-0.0.3-blue-geo-scale.png
	bin/badge --shield "Version-0.0.3-blue" --dark --shield_geometry "+0+25%" --shield_scale 0.75 --glob "/assets/icon175x175_fitrack_shield_Version-0.0.3-blue-geo-scale.png"
	cp $(testflight_icon) assets/icon175x175_shield_Version-0.0.3-blue-geo-scale.png
	bin/badge --shield "Version-0.0.3-blue" --dark --shield_geometry "+0+25%" --shield_scale 0.75 --glob "/assets/icon175x175_shield_Version-0.0.3-blue-geo-scale.png"

build:
	@echo "--> Make release"
	gem build badge.gemspec

test:
	@ruby -I. -rfind -e 'Find.find("test").each {|path| require path if path.end_with?("_test.rb")}'
