Vagrant.configure("2") do |config|
  config.vm.define "ubuntu2004" do |ubuntu2004|
	  config.vm.box = "generic/ubuntu2004"
	  ubuntu2004.vm.provision "shell", inline: "sudo apt-get install -y build-essential"
	  config.vm.synced_folder "./", "/home/vagrant/src", type: "rsync", rsync__exclude: ".git/"
  end

  config.vm.define "ubuntu2110" do |ubuntu2110|
	  config.vm.box = "generic/ubuntu2110"
	  ubuntu2110.vm.provision "shell", inline: "sudo apt-get install -y build-essential"
	  config.vm.synced_folder "./", "/home/vagrant/src", type: "rsync", rsync__exclude: ".git/"
  end
end
