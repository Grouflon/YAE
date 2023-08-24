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
		.exclude("src/yae/rendering/renderers/**/*.*")

	_settings[:defines] += [
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "yae.dll"
		_settings[:linker_flags] += ["-shared"]
		_settings[:defines] += ["_MT", "_DLL"]
		renderer = :opengl

		_settings[:include_dirs] += [
			"extern/gl3w/include",
		]

		_settings[:lib_dirs] += [
			"extern/SDL/lib/#{_settings[:platform]}",
			"extern/GLFW/lib/#{_settings[:platform]}",
		]

		_settings[:libs] += [
			"core",
			"SDL2",
			"delayimp",
		]
		_settings[:linker_flags] += ["-Xlinker /delayload:core.dll -Xlinker /delay:unload"]

		_settings[:defines] += [
			"IMGUI_API=__declspec(dllexport)",
			"GL3W_API=__declspec(dllexport)",
			"YAE_API=__declspec(dllexport)",
			"CORE_API=__declspec(dllimport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "yae.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
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
		"extern/imgui/imgui_widgets.cpp" => ["-Wno-unused-variable"],
		"extern/im3d/im3d.cpp" => ["-Wno-unused-variable", "-Wno-unused-function"],
	})

	_settings
end

task :link => ["core:build"]
define_module_tasks(settings())

end # namespace yae
