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

## Configuração da Pasta Compartilhada

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
## Atualização do Sistema e Dependências Nativas
Antes de iniciar a instalação dos componentes do Quartus, é necessário garantir que o sistema esteja atualizado e com as bibliotecas nativas e ferramentas essenciais de compilação instaladas.

```bash
sudo apt update
sudo apt install -y build-essential libglib2.0-0 libpng16-16 libfreetype6 libsm6 libice6 libxext6 libxrender1 libfontconfig1 libgl1-mesa-glx libxcb1 libx11-xcb1 libxi6 libxkbcommon0 libdbus-1-3 wget curl
```

## Configuração de Variável de Ambiente (`QUARTUS_CPUID_BYPASS`)
O Quartus possuem verificações e restrições relativas à arquitetura do processador (CPUID). Adicionar esta variável de ambiente ignora essa verificação.

```bash
echo 'export QUARTUS_CPUID_BYPASS=1' >> ~/.bashrc
source ~/.bashrc
```

## Habilitação de Arquitetura `amd64` e Emulação 
Posteriormente, o Rosetta 2 será habilitado para obter um desempenho superior. No entanto, nesta etapa inicial do processo de instalação, o instalador do Quartus funciona melhor sob o QEMU. Isso ocorre porque o **instalador gráfico** possui dependências legadas de 32 bits (i386/x86) e rotinas específicas que funcionam de forma mais estável e compatível com a emulação do QEMU/binfmt.

```bash
sudo dpkg --add-architecture amd64
sudo apt update
sudo apt install -y qemu-user-static binfmt-support
```

## Instalação das Bibliotecas de Compatibilidade Multi-Arquitetura (`amd64`)
Por fim, instalamos as versões de 64 bits (`amd64`) das bibliotecas gráficas e de sistema essenciais para que o executável x86_64 do Quartus possa rodar perfeitamente no ambiente emulado.

```bash
sudo apt update
sudo apt install -y libc6:amd64 libglib2.0-0:amd64 libpng16-16:amd64 libfreetype6:amd64 libsm6:amd64 libice6:amd64 libxext6:amd64 libxrender1:amd64 libfontconfig1:amd64 libgl1-mesa-glx:amd64 libxcb1:amd64 libx11-xcb1:amd64 libxi6:amd64 libxkbcommon0:amd64 libdbus-1-3:amd64
```
## Instalação do Quartus (versão 18.1)

Baixe o instalador do [Intel Quartus Prime Lite](https://www.altera.com/downloads/fpga-development-tools/quartus-prime-lite-edition-design-software-version-18-1-linux) em `.tar` para linux. Com o instalador na VM, extraia o `.tar`:

```bash
tar -xvf Quartus-lite-18.1.0.625-linux.tar
```

Dê permissão ao script de instalação e inicie-o:

```bash
chmod +x setup.sh
./setup.sh
```

O **instalador gráfico** irá abrir, e siga com as instruções. Marque no final a opção de criar um atalho no desktop.

> **Nota**: após a instalação o Quartus tentará abrir sem sucesso, não se preocupe, ainda tem mais alterações a serem feitas.

### Correção da Biblioteca Legada `libpng12` (Pós-Instalação)
O Intel Quartus Prime 18.1 depende de uma versão antiga da biblioteca PNG (`libpng12.so.0`) para renderizar ícones e componentes da sua interface gráfica. Como o Debian moderno não disponibiliza mais esse pacote nos repositórios oficiais, extraímos manualmente o binário `amd64` da biblioteca e o colocamos diretamente no diretório de binários do Quartus.

```bash
wget http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1.1_amd64.deb -O libpng12_amd64.deb
mkdir -p /tmp/libpng
dpkg-deb -x libpng12_amd64.deb /tmp/libpng/
cp /tmp/libpng/lib/x86_64-linux-gnu/libpng12.so.0.54.0 /home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/linux64/libpng12.so.0
```

### Ajuste do Atalho na Área de Trabalho (Desktop)
No fim da instalação, criamos um atalho na área de trabalho. Para garantir que o aplicativo abra carregando a variável de ambiente necessária (`QUARTUS_CPUID_BYPASS=1`) e apontando diretamente para o caminho das bibliotecas do Quartus (`LD_LIBRARY_PATH`), é necessário modificar a linha de execução (`Exec`) desse atalho específico.

```bash
sed -i 's|^Exec=.*|Exec=env QUARTUS_CPUID_BYPASS=1 LD_LIBRARY_PATH=/home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/linux64 /home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/linux64/quartus|' ~/Desktop/"Quartus (Quartus Prime 18.1) Lite Edition.desktop"
chmod +x ~/Desktop/"Quartus (Quartus Prime 18.1) Lite Edition.desktop"
```

Pronto, o aplicativo já abre com 2 cliques! Agora é necessário, ativar o Rosetta 2 para garantir um desempenho superior e possibilitar compilações.

## Configuração do Rosetta 2 no Debian

Para montar a unidade compartilhada do Rosetta (disponibilizada pelo framework de virtualização do macOS) e registrá-la para traduzir os binários x86_64/i386 em tempo de execução.

Primeiro crie um diretório e monte o virtiofs do Rosetta:
```bash
sudo mkdir -p /media/rosetta
sudo mount -t virtiofs rosetta /media/rosetta
```

Em seguida, registre o Rosetta no binfmt_misc para arquivos ELF x86_64:
```bash
echo -1 | sudo tee /proc/sys/fs/binfmt_misc/rosetta 2>/dev/null || true
echo ':rosetta:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xfe\xfe\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfb\xff\xff\xff:/media/rosetta/rosetta:CF' | sudo tee /proc/sys/fs/binfmt_misc/register
```
Por fim, habilite o Rosetta no update-binfmts:
```bash
sudo update-binfmts --enable rosetta
```

Para garantir que o Rosetta seja montado automaticamente no boot, temos que registrar o ponto de montagem no `/etc/fstab`, no terminal:

```bash
sudo nano /etc/fstab
```

Adicione a seguinte linha no final do arquivo:

```text
rosetta /media/rosetta virtiofs rosetta,nofail,defaults 0 0
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar. Agora vamos criar o script de montagem no `/etc/rc.local`. Como o driver do Rosetta finaliza seu carregamento apenas no fim do boot, o `rc.local` garante a montagem efetiva da pasta.

```bash
sudo nano /etc/rc.local
```
Cole o seguinte conteúdo:
```text
#!/bin/sh -e

mount -t virtiofs rosetta /media/rosetta

if [ -f /proc/sys/fs/binfmt_misc/rosetta ]; then
    echo -1 > /proc/sys/fs/binfmt_misc/rosetta 2>/dev/null || true
fi

echo ':rosetta:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xfe\xfe\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfb\xff\xff\xff:/media/rosetta/rosetta:CF' > /proc/sys/fs/binfmt_misc/register

exit 0
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar. E dê permissão de execução ao arquivo:

```bash
sudo chmod +x /etc/rc.local 
```

Agora **reinicie** a máquina virtual.

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

## Correção do SQLite3 para o Quartus
Para evitar falhas de segmentação ou problemas de dependência com o SQLite nativo durante o Map/Fit do Quartus, um 
wrapper C personalizado (`sqlite_wrapper.c`) foi criado para interceptar e envelopar as chamadas da biblioteca libsqlite3. 

Antes disso, instalamos o compilador cruzado para a arquitetura alvo (`gcc-x86-64-linux-gnu`) e os arquivos de cabeçalho e desenvolvimento da biblioteca SQLite64 (`libsqlite3-dev:amd64`). Isso permite compilar binários e bibliotecas dinâmicas x86_64 nativamente a partir do ambiente Debian `ARM64`.

```bash
sudo apt update
sudo apt install gcc-x86-64-linux-gnu libsqlite3-dev
sudo apt install libsqlite3-dev:amd64
```

Agora baixe o arquivo `sqlite_wrapper.c` nesse repositório, passe para a VM e cole no terminal:
```bash
x86_64-linux-gnu-gcc -shared -fPIC -o libccl_sqlite3.so sqlite_wrapper.c -I/usr/include -L/usr/lib/x86_64-linux-gnu -lsqlite3
cp libccl_sqlite3.so ~/intelFPGA_lite/18.1/quartus/linux64/
```
## Variáveis de Ambiente

Para garantir que os binários sejam encontrados e executados na arquitetura correta, as seguintes variáveis foram 
adicionadas ao final do arquivo `~/.bashrc`.

Abra o arquivo `~/.bashrc` com um editor de texto (como o `nano`):
```bash
nano ~/.bashrc
```

Cole as seguintes linhas no final do arquivo:

```bash
export QSYS_ROOTDIR="/home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/sopc_builder/bin"
export QENV_DISABLE_AVX=1
export MALLOC_CHECK_=0
export PATH=$PATH:/home/SEU_USUARIO/intelFPGA_lite/18.1/quartus/bin
export MTI_VCO_MODE=32
export PATH=/home/SEU_USUARIO/intelFPGA_lite/18.1/modelsim_ase/bin:$PATH
```
Aperte `control + O`, `return` e `control + X`, para sair e salvar.

Após adicionar rode:
```bash
source ~/.bashrc
```

## Correção do Script de Ambiente do Quartus (`qenv.sh`)
Para permitir que o script de inicialização rode dentro da VM ARM64 sem abortar, alteramos as regras de detecção de arquitetura no arquivo `qenv.sh`.

0. **Libere permissão de escrita e abra o arquivo:**
```bash
chmod +w ~/intelFPGA_lite/18.1/quartus/adm/qenv.sh
nano ~/intelFPGA_lite/18.1/quartus/adm/qenv.sh
```
1. **Injete a detecção de arquitetura ARM64:**
Localize a linha que contém `# We don't support processors without SSE extensions` e insira o seguinte bloco de código logo acima dela:
```bash
if test `uname -m` = "aarch64" ; then
    export QUARTUS_BIT_TYPE=64
fi
```
2. **Bypasse a validação de processador x86:**
Comente (adicione `#` no início das linhas) todo o bloco de código localizado entre a linha:

`# We don't support processors without SSE extensions...`

E a linha:

`##### Determine what bitness executables...`

3. **Salvar e fechar:**
Aperte `control + O`, `return` e `control + X`, para sair e salvar.

## Patch de Compatibilidade do ModelSim 
O ModelSim (ASE) possui um script interno chamado `vco` que gerencia o ambiente de execução e a detecção do kernel Linux. Em distribuições modernas (e sob ambientes ARM/emulação), esse script falha ao tentar localizar binários de 32 bits. Para liberar a edição do script e aplicar as correções:

```bash
chmod +w ~/intelFPGA_lite/18.1/modelsim_ase/vco
nano ~/intelFPGA_lite/18.1/modelsim_ase/vco
```
### Tabela de Modificações no Arquivo `vco`

| Linha / Trecho | Código Original | Código Modificado | Ponto de Edição |
| :---: | :---: | :---: | :---: |
| **Definição de Arquitetura** | `mode=${MTI_VCO_MODE:-""}` | `mode=${MTI_VCO_MODE:-"32"}` | <img width="721" height="452" alt="GNU nano 5 4" src="https://github.com/user-attachments/assets/3f076f04-d886-4717-87fe-ff508b1176dc" /> |
| **Injeção de Bibliotecas** | *(Abaixo de `` dir=`dirname "$arg0"` ``)* | `export LD_LIBRARY_PATH=${dir}/lib32:$LD_LIBRARY_PATH` | <img width="722" height="456" alt="File Edit View Terminal Tabs Heig" src="https://github.com/user-attachments/assets/b6bad7cc-8e68-456e-b8c6-77ec6d420c30" /> |
| **Tratamento do Kernel** | `*) vco="linux_rh60" ;;` | `*) vco="linux" ;;` | <img width="723" height="456" alt="Terminal - raphael-debian" src="https://github.com/user-attachments/assets/31567467-7fe0-42e8-bbca-d709327fe1b7" /> |

Aperte `control + O`, `return` e `control + X`, para sair e salvar.

## Correção da Interface Gráfica do ModelSim (FreeType)
O ModelSim exige uma versão específica legada do FreeType. Foi necessário compilar a versão `2.4.12` a partir do código-fonte com flags de 32-bits. Primeiro, instale o GCC e ferramentas de build para 32-bits:
```bash
sudo apt install -y gcc:i386 g++:i386 make:i386 curl tar bzip2
```

Agora baixe o código-fonte do FreeType 2.4.12 original e faça a compilação direcionada para a arquitetura i686:
```bash
cd ~/Downloads
curl -L -o freetype-2.4.12.tar.bz2 "https://downloads.sourceforge.net/project/freetype/freetype2/2.4.12/freetype-2.4.12.tar.bz2"
tar -xjvf freetype-2.4.12.tar.bz2
cd freetype-2.4.12
./configure --build=i686-pc-linux-gnu CC="gcc" "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
make -j$(nproc)
```

Em seguida, remova qualquer diretório lib32 criado anteriormente para evitar conflitos de permissão, crie a pasta dedicada do ModelSim e copie os arquivos gerados:
```bash
sudo rm -rf ~/intelFPGA_lite/18.1/modelsim_ase/lib32
mkdir -p ~/intelFPGA_lite/18.1/modelsim_ase/lib32
cp objs/.libs/libfreetype.so.6* ~/intelFPGA_lite/18.1/modelsim_ase/lib32/
ln -sf /usr/lib/i386-linux-gnu/libfontconfig.so.1 ~/intelFPGA_lite/18.1/modelsim_ase/lib32/
sudo chown -R $USER:$USER ~/intelFPGA_lite/18.1/modelsim_ase/lib32
```

Por fim, force o ModelSim a carregar a nova pasta lib32 antes das bibliotecas do sistema adicionando a variável no seu ~/.bashrc:
```bash
echo 'export LD_LIBRARY_PATH=$HOME/intelFPGA_lite/18.1/modelsim_ase/lib32:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

Pronto! Com essas correções aplicadas, o ambiente está totalmente configurado e pronto para você compilar seus projetos no Quartus Prime e rodar as simulações no ModelSim sem erros de arquitetura ou falhas de segmentação.

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
