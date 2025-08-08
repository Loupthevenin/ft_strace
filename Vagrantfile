Vagrant.configure("2") do |config|
  host_os = RbConfig::CONFIG['host_os']
  host_cpu = RbConfig::CONFIG['host_cpu']

  if host_os =~ /darwin/ && host_cpu == "arm64"
    box = "bento/ubuntu-22.04"
  else
    box = "ubuntu/trusty64"
  end
  provider = "virtualbox"
  puts "🛠️  Vagrant is using provider: #{provider}, with box: #{box}"

  config.vm.box = box
  config.vm.provider provider do |vb|
    vb.name = "ft_strace"
    vb.memory = 2048
  end

  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get install -y \
      gcc \
      g++ \
      make \
      libc6-dev \
      libc6-dev-i386 \
      libpthread-stubs0-dev \
      gcc-multilib \
      g++-multilib \
      elfutils \
      binutils \
      file \
      gdb \
      inetutils-ping \
      net-tools \
      procps \
      vim \
      strace \
      ltrace
      SHELL
end

