local dbug = require "dbug"

local display = {}

function display.getWidth()
	local tmpfile = "/tmp/tmvtmp"
	os.execute("tput cols > "..tmpfile)
	local file = io.open(tmpfile)
	if not file then dbug.error("could not get display width") end
	io.input(file)
	local width = io.read()
	file:close()
	return tonumber(width)
end

function display.getHeight()
	local tmpfile = "/tmp/tmvtmp"
	os.execute("tput lines > "..tmpfile)
	local file = io.open(tmpfile)
	if not file then dbug.error("could not get display height") end
	io.input(file)
	local height = io.read()
	file:close()
	return tonumber(height)
end

return display
