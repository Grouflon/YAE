namespace "yae" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
			"src/yae/**/*.cpp",
			"extern/im3d/*.cpp",
		]
		.exclude("src/yae/rendering/renderers/**/*.*")
		.exclude("src/yae/test/**/*.*")

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
		]
		_settings[:linker_flags] += []

		_settings[:defines] += [
			"IMGUI_USER_CONFIG=\\\"yae/yae_imconfig.h\\\"",

			"YAE_API=__declspec(dllexport)",
			"CORE_API=__declspec(dllimport)",
			"GL3W_API=__declspec(dllimport)",
			"IMGUI_API=__declspec(dllimport)",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "yae.wasm"
		_settings[:linker_flags] += ["-s SIDE_MODULE=1"]
		renderer = :opengl
	end

	# renderer
	if renderer == :opengl
		_settings[:source_files] |= FileList["src/yae/rendering/renderers/opengl/**/*.cpp"]
		_settings[:defines] += ["YAE_IMPLEMENTS_RENDERER_OPENGL=1"]
	end

	if _settings[:config] == "Release"
		with_test = false
		with_editor = false
	else
		with_test = true
		with_editor = true
	end

	# tests
	if with_test
		_settings[:source_files] |= FileList["src/yae/test/**/*.cpp"]
		_settings[:defines] += ["YAE_TESTS=1"]
	end

	# editor
	if with_editor
		_settings[:source_files] |= FileList["src/yae/editor/**/*.cpp"]
		_settings[:defines] += ["YAE_EDITOR=1"]
	end

	# file overrides
	_settings[:file_flags] = _settings[:file_flags].merge({
		"src/yae/FileWatchSystem.cpp" => ["-Wno-missing-field-initializers"], # incorrect initialization in FileWatch.hpp
		"extern/imgui/imgui_widgets.cpp" => ["-Wno-unused-variable"],
		"extern/im3d/im3d.cpp" => ["-Wno-unused-variable", "-Wno-unused-function"],
	})

	_settings
end

task :link => ["core:build"]
define_module_tasks(settings())

end # namespace yae
