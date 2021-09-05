Vagrant.configure("2") do |config|
  config.vm.box = "generic/ubuntu2004"
  config.vm.synced_folder "./", "/home/vagrant/src", type: "rsync", rsync__exclude: ".git/"
end
