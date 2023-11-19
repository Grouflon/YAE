PLATFORM = ENV["PLATFORM"]
CONFIG = ENV["CONFIG"]
TARGET = "#{PLATFORM}_#{CONFIG}"
BIN_DIR = "bin/#{TARGET}" # bin directory for all output binaries
OBJ_DIR = "intermediate/build/#{TARGET}/obj" # intermediate directory for generated object files
DEP_DIR = "intermediate/build/#{TARGET}/dep" # intermediate directory for generated dependency files


def default_settings()
	# GLOBALS
	platform = PLATFORM
	config = CONFIG
	bin = ""

	# COMPILER & FLAGS
	if platform == "Win64"
		c_compiler = "clang"
		cpp_compiler = "clang++"
		linker = "clang++"
	elsif platform == "Web"
		c_compiler = "emcc"
		cpp_compiler = "emcc"
		linker = "emcc"
	end
	c_flags = %W(-std=c11)
	cpp_flags = %W(-std=c++17)
	compiler_flags = [
		"-Wall",
		"-Werror",
		"-Wextra",
		"-Wno-unused-parameter",
		"-Wno-unused-function",
		"-Wno-gnu-anonymous-struct",
		"-Wno-nullability-completeness",
		"-Wno-nullability-extension",
		"-Wno-void-pointer-to-int-cast",
		"-Wno-int-to-void-pointer-cast",
		"-Wno-switch",

		"-g ", #generate symbols
	]
	dep_flags = "-MD -MF %{dep}" # flags required for dependency generation; passed to compilers
	linker_flags = [
		"-g ", #generate symbols
	]

	# INCLUDES
	include_dirs = [
		"src/",
		"extern/",
		"extern/mirror/",
		"extern/glm/",
		"extern/SDL/include/",
		"extern/imgui/",
		"extern/im3d/",
		"extern/glm/",
		"extern/stb/",
	]

	# DEFINES
	defines = [
		"_CRT_SECURE_NO_WARNINGS",
		"GLM_FORCE_RADIANS",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"IM3D_CONFIG=\\\"yae/yae_im3d_config.h\\\"",
		"MIRROR_EXTENSION_FILE=\\\"core/mirror_extension.h\\\"",
		"YAE_CONFIG=\\\"#{config}\\\"",
	]

	# LIBS
	libs = []
	lib_dirs = [
		"#{BIN_DIR}",
	]

	# SOURCES
	source_files = []

	# == CONFIG SPECIFIC
	if config == "Debug"
		compiler_flags += [
			"-O0",
		]
		linker_flags += [
			"-O0",
		]
		defines += [
			"DEBUG",
			"_DEBUG",
			"YAE_DEBUG",
		]
	elsif config == "Debug Optimized"
		compiler_flags += [
			"-O3",
		]
		linker_flags += [
			"-O3",
		]
		defines += [
			"NDEBUG",
			"YAE_DEBUG",
		]
	elsif config == "Release"
		compiler_flags += [
			"-O3",
			"-Wno-unused-comparison",
			"-Wno-unused-variable",
			"-Wno-unused-but-set-variable", # happens a lot with deactivated asserts
		]
		linker_flags += [
			"-O3",
		]
		defines += [
			"NDEBUG",
			"YAE_RELEASE",
		]
	end

	# == PLATFORM SPECIFIC
	if platform == "Win64"
		libs += [
			"user32",
			"kernel32",
			"gdi32",
			"imm32",
			"shell32",
		]

		defines += [
			"YAE_PLATFORM_WINDOWS",
		]

		linker_flags += [
			"-Xlinker /NODEFAULTLIB",
			"-Xlinker /ignore:4099", # Warning about missing pdbs for external libs, we don't care
		]

		if config == "Debug"
			libs += [
				"msvcrtd",
				"ucrtd",
				"vcruntimed",
				"msvcprtd",
			]
		else
			libs += [
				"msvcrt",
				"ucrt",
				"vcruntime",
				"msvcprt",
			]
		end
	elsif platform == "Web"
		compiler_flags += [
			"-Wno-unknown-pragmas",

			"-fPIC", # Position Independent Code
		]

		linker_flags += [
			"-s USE_SDL=2",
			"-s MIN_WEBGL_VERSION=2",
			"-s MAX_WEBGL_VERSION=2",
			"-gsource-map",
			"--source-map-base=http://localhost:6931/bin/Web_Debug", # allow the web browser debugger to have c++ functions information 
		]

		defines += [
			"YAE_PLATFORM_WEB"
		]
	end

	# FILE OVERRIDES
	file_flags = {}

	{
		:platform => platform,
		:config => config,
		:bin => bin,
		:c_compiler => c_compiler,
		:cpp_compiler => cpp_compiler,
		:linker => linker,
		:c_flags => c_flags,
		:cpp_flags => cpp_flags,
		:compiler_flags => compiler_flags,
		:dep_flags => dep_flags,
		:linker_flags => linker_flags,
		:include_dirs => include_dirs,
		:defines => defines,
		:libs => libs,
		:lib_dirs => lib_dirs,
		:source_files => source_files,
		:file_flags => file_flags,
	}
end

# FUNCTIONS
def src_to_obj(_s)
	_s.ext("cpp").prepend("#{OBJ_DIR}/")
end

def src_to_dep(_s)
	_s.ext("d").prepend("#{DEP_DIR}/")
end

def obj_to_src(_s, _settings)
	stem = _s.sub("#{OBJ_DIR}/", "")
	_settings[:source_files].detect{|_f| _f.ext("") == stem.ext("")}
end

def object_files(_settings)
	_settings[:source_files].clone.map{|_s| "#{OBJ_DIR}/#{_s.ext("o")}"}
end

def object_dependencies(_object_path, _settings)
	#sh "echo #{_object_path} #{_settings}"
	src_file = obj_to_src(_object_path, _settings)
	dep_file = src_to_dep(src_file)

	if File.exist?(dep_file)
		text = File.open(dep_file).read
	    text.gsub!(/(?<!\\)\ \\/, "") # remove trailing escaping backslash
	    text.gsub!(/^\ {2}/, '') # remove padding at the beginning of the line
	    text.gsub!(/(?<!\\)\ /, "\n") # put a line break at all the non escaped spaces
	    text.gsub!("\\ ", " ") # remove useless escaping slashes
	    text.gsub!("\\", "/") # reverse slashes

 	    text = text.split(/\n/).drop(1) # split all lines and remove .o at the first position

	    text
	else
		src_file
	end
end

def binary_file(_settings)
	"#{BIN_DIR}/#{_settings[:bin]}"
end

def _compile(_target, _settings)
	src_file = obj_to_src(_target, _settings)

	sh "echo Compiling #{src_file}...", verbose: false

	depflags = _settings[:dep_flags] % {target: _target, dep: src_to_dep(src_file)}
	compiler_flags = _settings[:compiler_flags].dup()
	if _settings[:file_flags][src_file]
		compiler_flags |= _settings[:file_flags][src_file]
	end
	includedirs_flags = _settings[:include_dirs].collect{|_s| "-I"+_s}.join(" ")
	defines_flags = _settings[:defines].collect{|_s| "-D"+_s}.join(" ")

	extension = File.extname(src_file)
	if extension == ".c"
		compiler = "#{_settings[:c_compiler]} #{_settings[:c_flags].join(" ")}"
	else
		compiler = "#{_settings[:cpp_compiler]} #{_settings[:cpp_flags].join(" ")}"
	end
	sh "#{compiler} #{compiler_flags.join(" ")} #{includedirs_flags} #{defines_flags} #{depflags} -c -o #{_target} #{src_file}", verbose: false
end

def _link(_settings)
	target = binary_file(_settings)
	objects = object_files(_settings)

	sh "echo Linking #{target}...", verbose: false

	libdirs_flags = _settings[:lib_dirs].collect{|_s| "-L"+_s}.join(" ")
	libs_flags = _settings[:libs].collect{|_s| "-l"+_s}.join(" ")

	sh "#{_settings[:linker]} #{_settings[:linker_flags].join(" ")} #{libdirs_flags} #{libs_flags} -o #{target} #{objects.join(" ")}", verbose: false
end

def _create_directories(_settings)
	#sh "echo Creating directories...", verbose: false

	directories = [BIN_DIR]
	_settings[:source_files].each do |_path|
		directories |= [File.dirname(src_to_obj(_path))]
		directories |= [File.dirname(src_to_dep(_path))]
	end

	directories.each do |_path|
		#sh "echo Creating dir #{_path}...", verbose: false
		FileUtils.mkdir_p _path
	end
end

def define_module_tasks(_settings)

	task :build => [:create_directories, :compile, :link]

	task :create_directories do |_task|	_create_directories(_settings) end
	multitask :compile => object_files(_settings)
	task :link => [binary_file(_settings)]

	file binary_file(_settings) => object_files(_settings) do |_task|
		_link(_settings)
	end

	obj = object_files(_settings)
	obj.each do |_obj|
		file _obj => object_dependencies(_obj, _settings) do |_task|
			_compile(_task.name, _settings)
		end
	end

end