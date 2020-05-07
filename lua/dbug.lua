local dbug = {}

function dbug.tprint(tbl, indent)
	if not indent then indent = 0 end
	for k, v in pairs(tbl) do
		formatting = string.rep("  ", indent) .. k .. ": "
		if type(v) == "table" then
			print(formatting)
			tprint(v, indent+1)
		elseif type(v) == 'boolean' then
			print(formatting .. tostring(v))
		else
			print(formatting .. v)
		end
	end
end

function dbug.log(message)
	print(
		"\27[33mLOG \27[39m(\27[32m"..debug.getinfo(2).name.."\27[39m) "..message
	)
end

function dbug.error(message)
	print(
		"\27[31mERROR \27[39m(\27[32m"..debug.getinfo(2).name.."\27[39m) "..message
	)
	dbug.exit()
end

function dbug.exit()
	dbug.log("exit")
	os.exit()
end

return dbug
