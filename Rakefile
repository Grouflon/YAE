require_relative "rake/_common"

require_relative "rake/core"
require_relative "rake/yae"
require_relative "rake/game"
require_relative "rake/main"

task :clean do
	sh "echo Cleaning #{TARGET}...", verbose: false
	FileUtils.remove_dir(BIN_DIR, true)
	FileUtils.remove_dir("intermediate/build/#{TARGET}", true)
end
