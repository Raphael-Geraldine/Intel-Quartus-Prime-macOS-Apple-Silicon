# Intel Quartus Prime em Apple Silicon

Este repositório documenta o processo e a configuração necessária para executar o **Intel Quartus Prime 18.1** (uma ferramenta tradicional e legada de desenvolvimento FPGA voltada para arquitetura x86_64) em um **MacBook Air M4 (Apple Silicon)** com **macOS Tahoe 26.5.2**.

<img width="1470" height="956" alt="QuartusNoMac" src="https://github.com/user-attachments/assets/86072088-9405-4d7d-a598-e359692524a2" />

Como o Quartus 18.1 não possui nativamente suporte para ARM e depende de binários x86_64, a solução implementada utiliza uma máquina virtual **Debian 11 ARM64** com o framework de virtualização do **Rosetta 2** integrado ao kernel do Linux. Essa abordagem permite a tradução e execução transparente de aplicativos x86_64 com alto desempenho diretamente no hardware Apple Silicon, viabilizando o fluxo de projeto, compilação e simulação para FPGAs de forma eficiente e moderna.

Para gravação, recomenda-se utilizar a ferramenta `openFPGAloader` que pode ser instalada diretamente no terminal do macOS via homebrew.
```zsh
brew install openfpgaloader
```
O que elimina a necessidade de configurar o _passthrough_ complexo de portas USB da máquina virtual ARM (Debian) para o hardware físico, algo que frequentemente falha com emuladores/hipervisores. Além de dispensar os drivers proprietários e legados da Intel (como o `jtagd` e os drivers do `USB-Blaster`), que foram projetados para arquiteturas x86 e causam incompatibilidades severas em ambientes virtualizados ou não nativos.

## Fluxo de Trabalho Recomendado

0. **Geração do Bitstream**: Realize a compilação e simulação do projeto no Quartus rodando no Debian 11 ARM com Rosetta 2.
1. **Transferência**: Compartilhe o arquivo de saída gerado (ex: projeto.sof) com o host macOS através de uma pasta compartilhada da VM.
2. **Gravação Nativa**: No terminal do macOS, com a placa conectada via USB, execute o comando de gravação:
```zsh
openfpgaloader -b <nome_da_sua_placa> caminho/para/o/arquivo.sof
```
O procedimento completo está detalhado nas seções a seguir.

> **Nota**: Este é o registro do processo que segui na minha máquina (MacBook Air M4, macOS Tahoe 26.5.2, UTM 4.7.5). Pode haver pequenas diferenças dependendo da sua configuração.

## Configuração da Máquina Virtual no UTM

Para hospedar o ambiente Debian 11 ARM no Apple Silicon, o **UTM** é o hipervisor recomendado, pois oferece excelente integração com os recursos de virtualização do macOS e suporte a aceleração de hardware.

### Criação e Configuração da VM

0. **Nova Máquina Virtual:** Instale e abra o [UTM](https://mac.getutm.app/) e escolha para criar uma nova VM, selecionando a opção para
virtualizar Linux e apontando para a imagem `.iso` do [Debian 11 ARM](https://cloud.debian.org/cdimage/archive/11.11.0/arm64/iso-cd/) (`debian-11.11.0-arm64-netinst.iso`).
Durante a instalação do Debian 11 na máquina virtual, recomenda-se selecionar o ambiente de desktop XFCE.
1. **Ativação do Rosetta:** Nas configurações da máquina virtual (na seção de sistema/arquitetura), ative a opção de **Rosetta** (suporte
a binários x86 em ARM). Esse recurso é indispensável, pois permite que o kernel do Debian traduza e execute as instruções x86_64 do Quartus
 18.1 diretamente, garantindo a compatibilidade do software da Intel.
2. **Diretório Compartilhado:** Configure uma **pasta compartilhada** nas opções de compartilhamento do UTM, selecionando
um diretório de sua preferência no host (macOS). Essa pasta facilitará o intercâmbio de arquivos, como os arquivos de projeto
e o *bitstream* final (`.sof`), entre o sistema hospedeiro e a máquina virtual.

> **Nota**: particularmente atribui 8 gb de memória ram, 4 cores do M4 e 64 gb de armazenamento, para o linux.  

## Habilitar "Copia e Cola"
Para habilitar a troca de textos e dados da área de transferência entre o macOS e o Debian, utiliza-se o agente do protocolo SPICE. Instale o pacote no Debian e ative o serviço:
```bash
sudo apt update
sudo apt install spice-vdagent
sudo systemctl enable --now spice-vdagent
```

Agora você pode copiar no seu Mac (usando o `command`) e colar no Debian (usando o `control`), e vice versa.
> Obs: lembrando que para copiar e colar no terminal do Debian deve-se pressionar a tecla `shift` também.

Agora **reinicie** a máquina virtual.

## Configuração da pasta compartilhada

Para garantir uma experiência de desenvolvimento fluida entre o host (Apple Silicon / macOS) e a máquina virtual
(Debian ARM no UTM), foram configurados o compartilhamento de arquivos via VirtioFS. O VirtioFS permite o compartilhamento de pastas de alta performance entre o host e a VM, mapeando diretamente o sistema de arquivos.

Para isso, crie o diretório de destino dentro do seu usuário no Debian:
```bash
mkdir -p /home/SEU_USUARIO/DebianShare
```
Adicione a linha abaixo no arquivo `/etc/fstab` para montar o diretório automaticamente durante o boot da máquina virtual:
```bash
sudo nano /etc/fstab
```
em seguida cole essa linha
```text
share /home/SEU_USUARIO/DebianShare virtiofs nofail,defaults 0 0
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar.

Para montar a pasta imediatamente sem precisar reiniciar, basta executar o seguinte comando no terminal do Debian:

```bash
sudo mount /home/SEU_USUARIO/DebianShare
```

## Configuração do Rosetta 2 no Debian

Primeiro, é necessário montar a unidade compartilhada do Rosetta (disponibilizada pelo framework de virtualização do macOS) e registrá-la para traduzir os binários x86_64/i386 em tempo de execução.

Primeiro crie um diretório e monte o virtiofs do Rosetta:
```bash
sudo mkdir -p /media/rosetta
sudo mount -t virtiofs rosetta /media/rosetta
```
Após isso, instale o suporte ao binfmt:
```bash
sudo apt update
sudo apt install binfmt-support
```

Em seguida, registre o Rosetta no binfmt_misc para arquivos ELF x86_64:
```bash
echo -1 | sudo tee /proc/sys/fs/binfmt_misc/rosetta 2>/dev/null || true
echo ':rosetta:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xfe\xfe\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfb\xff\xff\xff:/media/rosetta/rosetta:CF' | sudo tee /proc/sys/fs/binfmt_misc/register
```
Por fim, habilite o rosetta no update-binfmts:
```bash
sudo update-binfmts --enable rosetta
```

Para garantir que o Rosetta seja montado automaticamente no boot, adicione a seguinte linha ao `/etc/fstab`:
```bash
sudo nano /etc/fstab
```
em seguida cole essa linha
```text
rosetta /media/rosetta virtiofs rosetta,nofail,defaults 0 0
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar.

Além disso, desative o QEMU (caso esteja ativo) para garantir que o Rosetta assuma o controle em arquitetura x86_64:
```bash
echo -1 | sudo tee /proc/sys/fs/binfmt_misc/qemu-i386
```

## Adicionando Suporte a 32-bits (i386)
O ModelSim e várias ferramentas da suíte Quartus dependem fortemente de bibliotecas de 32-bits. Com isso, vamos habilitar a arquitetura i386:
```bash
sudo dpkg --add-architecture i386
sudo apt update
```
E instale as dependências necessárias:
```bash
sudo apt install -y \
  libbz2-1.0:i386 \
  libc6:i386 \
  libncurses5:i386 \
  libnss3:i386 \
  libstdc++6:i386 \
  libx11-6:i386 \
  libxext6:i386 \
  libxi6:i386 \
  libxft2:i386 \
  libxtst6:i386 \
  libxrender1:i386 \
  fontconfig:i386 \
  zlib1g:i386 \
  lib32z1 \
  lib32ncurses6
```
## Instalar dependências
Garanta que a arquitetura de 64-bits também esteja habilitada:
```bash
sudo dpkg --add-architecture amd64
sudo apt update
```

Execute o comando unificado abaixo para instalar todas as dependências de 64-bits para o Quartus e sua interface gráfica:

```bash
sudo apt install -y \
  libbz2-1.0:amd64 \
  libcrypt1:amd64 \
  libdbus-1-3:amd64 \
  libfontconfig:amd64 \
  libfontconfig1:amd64 \
  libfreetype6:amd64 \
  libgl1-mesa-glx:amd64 \
  libglib2.0-0:amd64 \
  libgtk-3-0:amd64 \
  libgtk2.0-0:amd64 \
  libice6:amd64 \
  libnsl-dev:amd64 \
  libpng16-16:amd64 \
  libsm6:amd64 \
  libusb-1.0-0:amd64 \
  libx11-xcb1:amd64 \
  libxcb1:amd64 \
  libxext6:amd64 \
  libxft2:amd64 \
  libxi6:amd64 \
  libxkbcommon0:amd64 \
  libxrender1:amd64 \
  libxtst6:amd64
```

Agora **reinicie** a máquina virtual.

## Instalação do Quartus (versão 18.1)

Baixe o instalador do [Intel Quartus Prime Lite](https://www.altera.com/downloads/fpga-development-tools/quartus-prime-lite-edition-design-software-version-18-1-linux) em `.tar` para linux. Com o instalador na VM, extraia o `.tar`:

```bash
tar -xvf Quartus-lite-18.1.0.625-linux.tar
```

### Modificação importantes para execução
Abra o arquivo `setup.sh` com um editor de texto (como o `nano`):
```bash
nano setup.sh
```
No topo do arquivo `setup.sh`, logo após a linha `#!/usr/bin/env bash`, insira isso:

```bash
if [ "$(uname -m)" = "aarch64" ]; then
    export QUARTUS_BIT_TYPE=64
fi
```

Agora no terminal:
```bash
export QUARTUS_CPUID_BYPASS=1
export QUARTUS_BIT_TYPE=64
./setup.sh
```

O instalador irá abrir, e siga com as instruções.

## Correção do FreeType para o ModelSim
O ModelSim exige uma versão específica legada do FreeType. Foi necessário compilar a versão `2.4.12` a partir do código-fonte com flags de 32-bits.
Baixa e extraia o FreeType 2.4.12:
```bash
cd ~/Downloads
curl -L -O https://download.savannah.gnu.org/releases/freetype/freetype-2.4.12.tar.bz2
tar -xjvf freetype-2.4.12.tar.bz2
cd freetype-2.4.12
```
Instale as dependências de build e compile em 32-bits:
```bash
sudo apt-get build-dep libfreetype6:i386
./configure --build=i686-pc-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
make -j8
```
Crie o diretório de bibliotecas 32-bits do ModelSim e faça os links simbólicos:
```bash
sudo mkdir -p /home/SEU_USUARIO/intelFPGA_lite/18.1/modelsim_ase/lib32
sudo ln -sf /usr/lib/i386-linux-gnu/libfreetype.so.6 /home/SEU_USUARIO/intelFPGA_lite/18.1/modelsim_ase/lib32/libfreetype.so.6
sudo ln -sf /usr/lib/i386-linux-gnu/libfontconfig.so.1 /home/SEU_USUARIO/intelFPGA_lite/18.1/modelsim_ase/lib32/libfontconfig.so.1
```
> **Nota**: Certifique-se de configurar as permissões corretas usando `sudo chown -R $USER:$USER` no diretório `lib32`.

## Correção do SQLite3 para o Quartus
Para evitar falhas de segmentação ou problemas de dependência com o SQLite nativo durante o Map/Fit do Quartus, um 
wrapper C personalizado (sqlite_wrapper.c) foi criado para interceptar e envelopar as chamadas da biblioteca libsqlite3. 
Baixe o arquivo nesse repositório, passe para a VM e cole no terminal:
```bash
x86_64-linux-gnu-gcc -shared -fPIC -o libccl_sqlite3.so sqlite_wrapper.c -I/usr/include -L/usr/lib/x86_64-linux-gnu -lsqlite3
cp libccl_sqlite3.so ~/intelFPGA_lite/18.1/quartus/linux64/
```

## Variáveis de Ambiente

Por fim, para garantir que os binários sejam encontrados e executados na arquitetura correta, as seguintes variáveis foram 
adicionadas ao final do arquivo `~/.bashrc`.

Abra o arquivo `~/.bashrc` com um editor de texto (como o `nano`):
```bash
nano ~/.bashrc
```

Cole as seguintes linhas no final do arquivo:

```bash
export QSYS_ROOTDIR="/home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/sopc_builder/bin"
export PATH=$PATH:/home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/bin
export PATH=/home/SEU_USUARIO/intelFPGA_lite/18.1/modelsim_ase/bin:$PATH
export MTI_VCO_MODE=32
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar.

Após adicionar rode:
```bash
source ~/.bashrc
```

## Comandos Úteis
Agora você pode rodar os fluxos do Quartus via terminal (CLI):

* **Compilar** (Sintetizar, Mapear e Fazer o Fit) um projeto:
```bash
quartus_map --read_settings_files=on nome_do_projeto
quartus_fit --read_settings_files=on nome_do_projeto
quartus_asm --read_settings_files=on nome_do_projeto
```

Tendo como resultado o arquivo `.sof` para poder subir na placa com o `openFPGAloader`.

* **Simular** por meio do ModelSim (VHDL):
```bash
vlib work
vcom projeto.vhd 
vcom tb_projeto.vhd 
vsim tb_projeto -do "add wave -radix binary *; run 100 ns; wave zoom full; quit"
```


<img width="1470" height="956" alt="TelaCheiaQuartusMac" src="https://github.com/user-attachments/assets/d6fbd427-ee68-4bec-8fdc-0c9b28e79a7d" />

### Sobre a licença do Intel Quartus Prime Lite Edition
O **Quartus Prime Lite Edition** é distribuído gratuitamente pela Intel/Altera, porém com uma ressalva importante: a licença gratuita cobre 
apenas uma lista específica de dispositivos suportados — famílias como **Cyclone**, **MAX 10** e **Arria II**, entre outras de entrada. 
Dispositivos de médio e alto desempenho (como as famílias **Arria 10**, **Stratix** e **Cyclone V GX/GT** em certas configurações) exigem uma 
licença paga das edições **Standard** ou **Pro**.

> Este repositório documenta apenas o processo de **virtualização e execução** da ferramenta em Apple Silicon. O uso do Quartus Prime está sujeito aos termos de licença da própria Intel/Altera, disponíveis em seu site oficial.
