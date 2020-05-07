local math = require "math"
local vips = require "vips"

local dbug = require "dbug"
local window = require "window"

local picture = {}

function picture.display(image)
	dbug.log("image dimensions: "..image:width().." * "..image:height())
	local width = image:width() - 1
	local height = image:height() - 1
	for x = 0, 10 do
		for y = 0, 10 do
			--local r1, g1, b1 = image(x, y)
			print(x..", "..y..": "..image(1, 1))
		end
		print("\n")
	end
	--print(image(0, 2))

end

function picture.load(width, height, input)
	dbug.log("target image: "..input)
	local image = vips.Image.new_from_file(input)
	dbug.log("raw image dimensions: "..image:width().." * "..image:height())
	dbug.log(
		"window dimensions: "..window.getWidth().." * "..window.getHeight()
	)

	if width and not height then
		height = width * (image:width() / image:height())
	elseif height and not width then
		width = height * (image:height() / image:width())
	elseif not width and not height then
		width = window.getWidth()
		height = window.getHeight()
	end

	-- http://libvips.github.io/libvips/API/current/libvips-resample.html#vips-thumbnail
	image = vips.Image.thumbnail(input, width, {height = height})

	dbug.log("resized image dimensions: "..image:width().." * "..image:height())

	picture.display(image)
end

return picture
