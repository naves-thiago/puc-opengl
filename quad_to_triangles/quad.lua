if not arg[1] or not arg[2] then
	print("Usage: lua quad.lua in_file.obj out_file.obj")
	os.exit(1)
end

fout = io.open(arg[2], "w")

for l in io.lines(arg[1]) do
	if l:sub(1, 2) ~= 'f ' then
		fout:write(l, '\n')
		goto continue
	end

	a, b, c, d = string.match(l, 'f (%d+/%d+/%d+) (%d+/%d+/%d+) (%d+/%d+/%d+) (%d+/%d+/%d+)')
	if d then
		fout:write('f ', a, ' ', b, ' ', c, '\n')
		fout:write('f ', c, ' ', d, ' ', a, '\n')
	else
		fout:write(l, '\n')
	end

	::continue::
end

fout:close()
