namespace "main" do

def settings()
	_settings = default_settings()
	_settings[:source_files] |= FileList[
		"src/main.cpp",
	]

	if _settings[:platform] == "Win64"
		_settings[:bin] = "main.exe"

		_settings[:libs] |= [
			"core",
		]

	elsif _settings[:platform] == "Web"
		_settings[:bin] = "main.html"
		_settings[:linker_flags] += [
			"-s MAIN_MODULE=1",
			"-s LLD_REPORT_UNDEFINED",
			"-s INITIAL_MEMORY=134217728",
			"-s ALLOW_MEMORY_GROWTH=1",
			"-s ASSERTIONS=1",
			"--preload-file ./data",
			"--preload-file #{BIN_DIR}@/",
			"--use-preload-plugins",
			"#{BIN_DIR}/yae.wasm",
			"--emrun",
		]
	end

	_settings
end
main_settings = settings()

task :copy_files

if PLATFORM == "Win64"
	task :copy_files => ["#{BIN_DIR}/SDL2.dll"]
	file "#{BIN_DIR}/SDL2.dll" => ["extern/SDL/lib/Win64/SDL2.dll"] do |_task|
		sh "echo Copying #{_task.source} -> #{_task.name}...", verbose: false
		FileUtils.copy_file(_task.source, _task.name)
	end
elsif PLATFORM == "Web"
	# relink when any of the other modules changes so that the program takes the modification (all files are embedded in the program)
	file binary_file(main_settings) => [ "#{BIN_DIR}/core.wasm", "#{BIN_DIR}/yae.wasm", "#{BIN_DIR}/test.wasm", "#{BIN_DIR}/editor.wasm", "#{BIN_DIR}/game.wasm"]
end

task :link => ["core:build", "yae:build", "game:build", :copy_files]
define_module_tasks(main_settings)

end # namespace main
