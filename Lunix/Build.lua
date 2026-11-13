local lfs = require("lfs")

local Output = "Lunix.bin"

local SourceFolders = {
	"Source",
}
local IncludeFolders = {}

local CFlags = "-Wall -Wextra"
local LDFlags = "-O1"

function ListAllFiles(Directory)
	local Files = {}

	for File in lfs.dir(Directory) do
		if File ~= "." and File ~= ".." then
			local FullPath = Directory .. "/" .. File
			local Attributes = lfs.attributes(FullPath)

			if Attributes.mode == "directory" then
				local SubFiles = ListAllFiles(FullPath)

				for _, SubFile in ipairs(SubFiles) do
					table.insert(Files, SubFile)
				end
			else
				if File:match("%.c$") then
					table.insert(Files, FullPath)
				end
			end
		end
	end

	return Files
end

function Run(Command)
	print(Command)

	local Success, Type, Code = os.execute(Command)
	if not Success then
		error(string.format("Command failed (exit code: %s): %s", tostring(Code), Command))
	end

	return Success
end

local BuildCommand = string.format(
	"%s ../sdk/aurcc.py build --cflags %q -o %q",
	os.getenv("PYTHON") or "python3",
	CFlags .. " " .. LDFlags,
	Output
)

for _, SourceFolder in ipairs(SourceFolders) do
	local Sources = ListAllFiles(SourceFolder)

	for _, CFile in ipairs(Sources) do
		BuildCommand = BuildCommand .. " " .. CFile
	end
end

Run(BuildCommand)
