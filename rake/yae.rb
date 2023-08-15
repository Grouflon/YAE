namespace "yae" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
			"src/yae/**/*.cpp",
			"extern/imgui/*.cpp",
			"extern/im3d/*.cpp",
			"extern/imgui/backends/imgui_impl_sdl.cpp",
			"extern/imgui/backends/imgui_impl_opengl3.cpp",
		]
		.exclude("src/yae/platforms/**/*.*")
		.exclude("src/yae/rendering/renderers/**/*.*")

	_settings[:defines] += [
		"_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING",
		"_LIBCPP_NO_EXPERIMENTAL_DEPRECATION_WARNING_FILESYSTEM",
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "yae.dll"
		_settings[:linker_flags] += ["-shared"]
		_settings[:defines] += ["_MT", "_DLL"]
		renderer = :opengl

		_settings[:source_files] |= FileList["src/yae/platforms/windows/**/*.cpp"]

		_settings[:include_dirs] += [
			"extern/dbghelp/inc/",
			"extern/gl3w/include",
		]

		_settings[:lib_dirs] += [
			"extern/GLFW/lib/#{_settings[:platform]}",
			"extern/SDL/lib/#{_settings[:platform]}",
			"extern/dbghelp/lib/x64/",
		]

		_settings[:libs] += [
			"SDL2",
			"dbghelp",
		]

		_settings[:defines] += [
			"MIRROR_API=__declspec(dllexport)",
			"IMGUI_API=__declspec(dllexport)",
			"GL3W_API=__declspec(dllexport)",
			"YAE_API=__declspec(dllexport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "yae.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
		_settings[:source_files] |= FileList["src/yae/platforms/web/**/*.cpp"]
		renderer = :opengl
	end

	if renderer == :opengl
		_settings[:source_files] |= FileList["src/yae/rendering/renderers/opengl/**/*.cpp"]
		_settings[:defines] += ["YAE_IMPLEMENTS_RENDERER_OPENGL=1"]

		_settings[:source_files] |= ["extern/gl3w/src/gl3w.c"]
		_settings[:include_dirs] += ["extern/gl3w/include"]
	end

	# file overrides
	_settings[:file_flags] = _settings[:file_flags].merge({
		"src/yae/ResourceManager.cpp" => ["-Wno-missing-field-initializers"],
		"src/yae/platforms/windows/windows_platform.cpp" => ["-Wno-extra-tokens", "-Wno-pragma-pack"],
		"extern/imgui/imgui_widgets.cpp" => ["-Wno-unused-variable"],
		"extern/im3d/im3d.cpp" => ["-Wno-unused-variable", "-Wno-unused-function"],
	})

	_settings
end

define_module_tasks(settings())

end # namespace yae
