local argparse = require "argparse"
local signal = require "posix.signal"
local vips = require "vips"

local dbug = require "dbug"
local format = require "format"
local window = require "window"
local picture = require "picture"

function parseArguments()
	local parser = argparse(
		"tmv v0.1",
		"View images and videos without leaving the console.\n"..
		"Requires a terminal that supports truecolor and utf-8",
		"For more info visit <https://github.com/kal39/TerminalMediaViewer>"
	)

	parser:argument("input", "Input file.")

	parser:group(
		"Configure output dimensions",
		parser:option("-W --width", "Set output width."),
		parser:option("-H --height", "Set output height.")
	)

	parser:group(
		"Configure fps",
		parser:option("-f --fps", "Set fps. Default 15 fps.", 15),
		parser:flag("-F", "Use original fps from video. Default 15 fps.")
	)

	parser:group(
		"Other options",
		parser:flag("-s --no-sound", "Disable sound.")
	)

	return parser:parse()
end

function main()
	signal.signal(signal.SIGINT, dbug.exit)

	args = parseArguments()

	dbug.log("target file: "..args.input)

	if io.open(args.input, "r") == nil then
		dbug.error(args.input.." does not exist")
	end

	extension = args.input:match("(%w+)$")

	dbug.log("target file extension: "..extension)

	if format.isVideo(extension) then
	else
		picture.load(tonumber(args.width), tonumber(args.height), args.input)
	end

	dbug.exit()
end

main()
