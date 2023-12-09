require_relative "rake/_common"

require_relative "rake/core"
require_relative "rake/yae"
require_relative "rake/game"
require_relative "rake/main"

task :clean do
	sh "echo Cleaning #{TARGET}...", verbose: false

	if Dir.exist?(BIN_DIR)
		FileUtils.remove_entry_secure(BIN_DIR, false)
	end
	
	intermediate_dir = "intermediate/build/#{TARGET}"
	if Dir.exist?(intermediate_dir)
		FileUtils.remove_entry_secure(intermediate_dir, false)
	end
end
