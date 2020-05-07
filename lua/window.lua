local dbug = require "dbug"

local window = {}

function window.getWidth()
	local tmpfile = "/tmp/tmvtmp"
	os.execute("tput cols > "..tmpfile)
	local file = io.open(tmpfile)
	if not file then dbug.error("could not get display width") end
	io.input(file)
	local width = io.read()
	file:close()
	return tonumber(width)
end

function window.getHeight()
	local tmpfile = "/tmp/tmvtmp"
	os.execute("tput lines > "..tmpfile)
	local file = io.open(tmpfile)
	if not file then dbug.error("could not get display height") end
	io.input(file)
	local height = io.read()
	file:close()
	return tonumber(height)
end

function window.setColor(r1, g1, b1, r2, g2, b2)
	print("\x1b[48;2;"..r1..";"..g1..";"..b1.."m")
	print("\x1b[38;2;"..r2..";"..g2..";"..b2.."m")
end

function window.resetColor() print("\x1b[0m") end


return window
