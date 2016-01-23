require "tundra.syntax.glob"
require "tundra.syntax.rust-osx-bundle"
require "tundra.syntax.rust-cargo"
require "tundra.path"
require "tundra.util"

local native = require "tundra.native"

-----------------------------------------------------------------------------------------------------------------------

local function get_rs_src(dir)
	return Glob {
		Dir = dir,
		Extensions = { ".rs" },
		Recursive = true,
	}
end

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "ui_testbench",
	CargoConfig = "src/prodbg/ui_testbench/Cargo.toml",
	Sources = { 
		get_rs_src("src/prodbg/ui_testbench"),
		get_rs_src("src/prodbg/core"),
		get_rs_src("src/ui"),
	},

    Depends = { "ui", "lua", "remote_api", "stb", "bgfx", "imgui", "scintilla", "tinyxml2", "capstone" },
}

-----------------------------------------------------------------------------------------------------------------------

RustProgram {
	Name = "prodbg",
	CargoConfig = "src/prodbg/main/Cargo.toml",
	Sources = { 
		get_rs_src("src/prodbg/main"),
		get_rs_src("src/prodbg/core"),
		get_rs_src("src/ui"),
	},

    Depends = { "ui", "lua", "remote_api", "stb", "bgfx", "imgui", "scintilla", "tinyxml2", "capstone" },
}


-----------------------------------------------------------------------------------------------------------------------

local prodbgBundle = RustOsxBundle 
{
	Depends = { "prodbg" },
	Target = "$(OBJECTDIR)/ProDBG.app",
	InfoPList = "Data/Mac/Info.plist",
	Executable = "prodbg",
	Resources = {
		CompileNib { Source = "data/mac/appnib.xib", Target = "appnib.nib" },
		"data/mac/icon.icns",
	},
}

-----------------------------------------------------------------------------------------------------------------------

if native.host_platform == "macosx" then
	Default(prodbgBundle)
else
	Default "prodbg"
end


Default "ui_testbench"
-- Default "prodbg"

