namespace "core" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
			"src/core/**/*.cpp",
		]
		.exclude("src/core/platforms/**/*.*")

	_settings[:defines] += [
		"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING",
		"_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM",
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "core.dll"
		_settings[:linker_flags] += ["-shared"]
		_settings[:defines] += ["_MT", "_DLL"]
		renderer = :opengl

		_settings[:source_files] |= FileList["src/core/platforms/windows/**/*.cpp"]

		_settings[:include_dirs] += [
			"extern/dbghelp/inc/",
			"extern/gl3w/include",
		]

		_settings[:lib_dirs] += [
			"extern/SDL/lib/#{_settings[:platform]}",
			"extern/dbghelp/lib/x64/",
		]

		_settings[:libs] += [
			"SDL2",
			"dbghelp",
			"delayimp",
		]

		_settings[:defines] += [
			"MIRROR_API=__declspec(dllexport)",
			"CORE_API=__declspec(dllexport)",
			"IMGUI_API=__declspec(dllexport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "core.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
		_settings[:source_files] |= FileList["src/core/platforms/web/**/*.cpp"]
	end

	# file overrides
	_settings[:file_flags] = _settings[:file_flags].merge({
		"src/core/platforms/windows/windows_platform.cpp" => ["-Wno-extra-tokens", "-Wno-pragma-pack"],
	})

	_settings
end

define_module_tasks(settings())

end # namespace core
