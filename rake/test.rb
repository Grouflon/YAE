namespace "test" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
		"src/test/**/*.cpp",
	]

	_settings[:compiler_flags] |= [
		"-Wno-unused-variable",
		"-Wno-unused-function",
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "test.dll"
		_settings[:linker_flags] += ["-shared"]
		_settings[:defines] += ["_MT", "_DLL"]

		_settings[:libs] |= [
			"yae",
		]

		_settings[:defines] += [
			"MIRROR_API=__declspec(dllimport)",
			"IMGUI_API=__declspec(dllimport)",
			"YAE_API=__declspec(dllimport)",
			"TEST_API=__declspec(dllexport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "test.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
	end

	_settings
end

task :link => ["yae:build"]
define_module_tasks(settings())

end # namespace test
