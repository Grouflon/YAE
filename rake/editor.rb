namespace "editor" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
		"src/editor/**/*.cpp",
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "editor.dll"
		_settings[:linker_flags] += ["-shared"]
		_settings[:defines] += ["_MT", "_DLL"]

		_settings[:libs] |= [
			"core",
			"yae",
			"delayimp",
		]
		_settings[:linker_flags] += ["-Xlinker /delayload:core.dll -Xlinker /delayload:yae.dll -Xlinker /delay:unload"]

		_settings[:defines] += [
			"CORE_API=__declspec(dllimport)",
			"MIRROR_API=__declspec(dllimport)",
			"IMGUI_API=__declspec(dllimport)",
			"YAE_API=__declspec(dllimport)",
			"EDITOR_API=__declspec(dllexport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "editor.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
	end

	_settings
end

task :link => ["yae:build"]
define_module_tasks(settings())

end # namespace editor
