# Intel Quartus Prime on Apple Silicon

> **Languages:** 🇺🇸 **English** or 🇧🇷 [Português](https://github.com/Raphael-Geraldine/Intel-Quartus-Prime-macOS-Apple-Silicon/blob/main/README.pt_br.md)

This repository documents the process and configuration required to run **Intel Quartus Prime 18.1** (a traditional and legacy FPGA development tool targeting x86_64 architecture) on a **MacBook Air M4 (Apple Silicon)** running **macOS Tahoe 26.5.2**.

<img width="1470" height="956" alt="QuartusNoMac" src="https://github.com/user-attachments/assets/86072088-9405-4d7d-a598-e359692524a2" />

Since Quartus 18.1 lacks native ARM support and relies on x86_64 binaries, the implemented solution uses a **Debian 11 ARM64** virtual machine with the **Rosetta 2** virtualization framework integrated directly into the Linux kernel. This approach enables a high-performance translation and execution of x86_64 applications directly on Apple Silicon hardware, making the FPGA design, compilation, and simulation workflow viable, efficient, and modern.

For bitstream loading, it is recommended to use the `openFPGAloader` tool, which can be installed directly in the macOS terminal via Homebrew.
```zsh
brew install openfpgaloader
```
This eliminates the need to configure complex USB port passthrough from the ARM virtual machine (Debian) to the physical hardware, something that frequently fails with emulators/hypervisors. In addition to eliminating the need for Intel's proprietary and legacy drivers (such as `jtagd` and `USB-Blaster` drivers), which were designed for x86 architectures and cause severe incompatibilities in virtualized or non-native environments.

## Recommended Workflow

0. **Bitstream Generation**: Compile and simulate the project in Quartus running on Debian 11 ARM with Rosetta 2.
1. **Transfer**: Share the generated output file (e.g., project.sof) with the macOS host through a VM shared folder.
2. **Native bitstream loading**: In the macOS terminal, with the board connected via USB, run the flashing command:
```zsh
openfpgaloader -b <your_board_name> path/to/file.sof
```
The complete procedure is detailed in the following sections.

> **Note**: This is the record of the process I followed on my machine (MacBook Air M4, macOS Tahoe 26.5.2, UTM 4.7.5). There may be slight differences depending on your setup.

## UTM Virtual Machine Setup

To host the Debian 11 ARM environment on Apple Silicon, **UTM** is the recommended hypervisor, as it offers excellent integration with macOS virtualization features.

### VM Creation and Configuration

0. **New Virtual Machine:** Install and open [UTM](https://mac.getutm.app/) and choose to create a new VM, selecting the option to virtualize Linux and pointing to the `.iso` image of [Debian 11 ARM](https://cloud.debian.org/cdimage/archive/11.11.0/arm64/iso-cd/) (`debian-11.11.0-arm64-netinst.iso`).
During the Debian 11 installation inside the virtual machine, selecting the XFCE desktop environment is recommended.
1. **Enabling Rosetta:** In the virtual machine settings (under the system/architecture section), enable the **Rosetta** option (support for x86 binaries on ARM). This feature is essential, as it allows the Debian kernel to directly translate and execute Quartus 18.1 x86_64 instructions, ensuring compatibility with Intel software.
2. **Shared Directory:** Configure a **shared folder** in UTM's sharing options, selecting a directory of your choice on the host (macOS). This folder will facilitate file exchanges, such as project files and the final bitstream (`.sof`), between the host system and the virtual machine.

> **Note**: I personally assigned 8 GB of RAM, 4 M4 cores, and 64 GB of storage to Linux.  

## Enabling "Copy and Paste"
To enable sharing text and clipboard data between macOS and Debian, the SPICE protocol agent is used. Install the package in Debian and enable the service:
```bash
sudo apt update
sudo apt install spice-vdagent
sudo systemctl enable --now spice-vdagent
```

Now you can copy on your Mac (using `command`) and paste in Debian (using `control`), and vice versa.
> Note: keep in mind that to copy and paste in the Debian terminal, you must also press the `shift` key..

Now **restart** the virtual machine.

## Shared Folder Configuration

To ensure a smooth development experience between the host (Apple Silicon / macOS) and the virtual machine (Debian ARM in UTM), file sharing was configured via VirtioFS. VirtioFS enables high-performance folder sharing between the host and the VM by mapping the filesystem directly.

To do this, create the target directory inside your user folder in Debian:
```bash
mkdir -p /home/YOUR_USERNAME/DebianShare
```
Add the line below to the `/etc/fstab` file to mount the directory automatically during virtual machine boot:
```bash
sudo nano /etc/fstab
```
then paste this line
```text
share /home/YOUR_USERNAME/DebianShare virtiofs nofail,defaults 0 0
```
Press `control + O`, `return` and `control + X` to save and exit.

To mount the folder immediately without needing to restart, simply run the following command in the Debian terminal:

```bash
sudo mount /home/YOUR_USERNAME/DebianShare
```
## System Update and Native Dependencies
Before starting the Quartus components installation, you must ensure the system is up to date and equipped with native libraries and essential build tools.

```bash
sudo apt update
sudo apt install -y build-essential libglib2.0-0 libpng16-16 libfreetype6 libsm6 libice6 libxext6 libxrender1 libfontconfig1 libgl1-mesa-glx libxcb1 libx11-xcb1 libxi6 libxkbcommon0 libdbus-1-3 wget curl
```

## Environment Variable Setup (`QUARTUS_CPUID_BYPASS`)
Quartus performs CPU architecture checks and restrictions based on CPUID. Adding this environment variable bypasses that check.

```bash
echo 'export QUARTUS_CPUID_BYPASS=1' >> ~/.bashrc
source ~/.bashrc
```

## Enabling `amd64` Architecture and Emulation
Later on, Rosetta 2 will be enabled to achieve superior performance. However, in this initial stage of the installation process, the Quartus installer works better under QEMU. This is because the **GUI installer** relies on legacy 32-bit (i386/x86) dependencies and specific routines that run more stably and compatibly under QEMU/binfmt emulation.

```bash
sudo dpkg --add-architecture amd64
sudo apt update
sudo apt install -y qemu-user-static binfmt-support
```

## Multi-Architecture (`amd64`) Compatibility Libraries Installation
Finally, we install the 64-bit (`amd64`) versions of essential graphics and system libraries so that the Quartus x86_64 executable can run seamlessly in the emulated environment.

```bash
sudo apt update
sudo apt install -y libc6:amd64 libglib2.0-0:amd64 libpng16-16:amd64 libfreetype6:amd64 libsm6:amd64 libice6:amd64 libxext6:amd64 libxrender1:amd64 libfontconfig1:amd64 libgl1-mesa-glx:amd64 libxcb1:amd64 libx11-xcb1:amd64 libxi6:amd64 libxkbcommon0:amd64 libdbus-1-3:amd64
```
## Quartus Installation (version 18.1)

Download the [Intel Quartus Prime Lite](https://www.altera.com/downloads/fpga-development-tools/quartus-prime-lite-edition-design-software-version-18-1-linux) `.tar` installer for Linux. Once the installer is inside the VM, extract the `.tar` file:

```bash
tar -xvf Quartus-lite-18.1.0.625-linux.tar
```

Grant execution permissions to the installation script and launch it:

```bash
chmod +x setup.sh
./setup.sh
```

The **GUI installer** will open; choose the default path and follow the instructions. At the end, check the option to create a desktop shortcut.

<table border="0" style="width: 100%;">
  <tr>
    <td width="50%" align="center">
      <img src="https://github.com/user-attachments/assets/ee384bf4-54ad-4d61-962c-d46efbe1661a" width="100%" />
    </td>
    <td width="50%" align="center">
      <img src="https://github.com/user-attachments/assets/2aa93535-dc29-4d6c-b14e-5dc0c876f3ca" width="100%" />
    </td>
  </tr>
</table>

> **Note**: After installation if you choose open Quartus, it won't be possible, do not worry, there are still more changes to be made.

### Fixing the Legacy `libpng12` Library (Post-Installation)
Intel Quartus Prime 18.1 relies on an older version of the PNG library (`libpng12.so.0`) to render icons and components in its graphical interface. Since modern Debian no longer provides this package in its official repositories, we manually extract the `amd64` binary of the library and place it directly into the Quartus binaries directory.

```bash
wget http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1.1_amd64.deb -O libpng12_amd64.deb
mkdir -p /tmp/libpng
dpkg-deb -x libpng12_amd64.deb /tmp/libpng/
cp /tmp/libpng/lib/x86_64-linux-gnu/libpng12.so.0.54.0 /home/YOUR_USERNAME/intelFPGA_lite/18.1/quartus/linux64/libpng12.so.0
```

### Adjusting the Desktop Shortcut
At the end of the installation, a desktop shortcut was created. To ensure the application launches with the required environment variable (`QUARTUS_CPUID_BYPASS=1`) and explicitly points to the Quartus library path (`LD_LIBRARY_PATH`), you need to modify the execution line (`Exec`) of this specific shortcut.

```bash
sed -i 's|^Exec=.*|Exec=env QUARTUS_CPUID_BYPASS=1 LD_LIBRARY_PATH=/home/YOUR_USERNAME/intelFPGA_lite/18.1/quartus/linux64 /home/YOUR_USERNAME/intelFPGA_lite/18.1/quartus/linux64/quartus|' ~/Desktop/"Quartus (Quartus Prime 18.1) Lite Edition.desktop"
chmod +x ~/Desktop/"Quartus (Quartus Prime 18.1) Lite Edition.desktop"
```

Done, the application now opens with a double click! Now it's necessary to enable Rosetta 2 to ensure superior performance and make compilations possible.

## Rosetta 2 Setup on Debian

To mount the Rosetta shared drive (provided by the macOS virtualization framework) and register it to translate x86_64 binaries at runtime.

First, create a directory and mount Rosetta's virtiofs:
```bash
sudo mkdir -p /media/rosetta
sudo mount -t virtiofs rosetta /media/rosetta
```

Next, register Rosetta in binfmt_misc for x86_64 ELF files:
```bash
echo -1 | sudo tee /proc/sys/fs/binfmt_misc/rosetta 2>/dev/null || true
echo ':rosetta:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xfe\xfe\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfb\xff\xff\xff:/media/rosetta/rosetta:CF' | sudo tee /proc/sys/fs/binfmt_misc/register
```
Finally, enable Rosetta in update-binfmts:
```bash
sudo update-binfmts --enable rosetta
```

To ensure Rosetta is mounted automatically on boot, we must register the mount point in `/etc/fstab`. In the terminal:

```bash
sudo nano /etc/fstab
```

Add the following line to the end of the file:

```text
rosetta /media/rosetta virtiofs rosetta,nofail,defaults 0 0
```
Press `control + O`, `return` and `control + X` to save and exit. Now let's create the mount script in `/etc/rc.local`. Since the Rosetta driver finishes loading only at the end of the boot process, `rc.local` ensures the folder is effectively mounted.

```bash
sudo nano /etc/rc.local
```
Paste the following content:
```text
#!/bin/sh -e

mount -t virtiofs rosetta /media/rosetta

if [ -f /proc/sys/fs/binfmt_misc/rosetta ]; then
    echo -1 > /proc/sys/fs/binfmt_misc/rosetta 2>/dev/null || true
fi

echo ':rosetta:M::\x7fELF\x02\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x3e\x00:\xff\xff\xff\xff\xff\xfe\xfe\x00\xff\xff\xff\xff\xff\xff\xff\xff\xfb\xff\xff\xff:/media/rosetta/rosetta:CF' > /proc/sys/fs/binfmt_misc/register

exit 0
```
Press `control + O`, `return` and `control + X` to save and exit. Then grant execution permissions to the file:

```bash
sudo chmod +x /etc/rc.local 
```

Now **restart** the virtual machine.

## Adding 32-bit (i386) Support
ModelSim and several tools within the Quartus suite rely heavily on 32-bit libraries. Therefore, we will enable the i386 architecture:
```bash
sudo dpkg --add-architecture i386
sudo apt update
```
And install the required dependencies:
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

## SQLite3 Fix for Quartus
To avoid segmentation faults or dependency issues with native SQLite during Quartus Map/Fit, a custom C wrapper (`sqlite_wrapper.c`) was created to intercept and wrap calls to the libsqlite3 library.

Before that, we install the cross-compiler for the target architecture (`gcc-x86-64-linux-gnu`) as well as the header and development files for the SQLite64 library (`libsqlite3-dev:amd64`). This allows compiling x86_64 binaries and dynamic libraries natively from the Debian ARM64 environment.

```bash
sudo apt update
sudo apt install gcc-x86-64-linux-gnu libsqlite3-dev
sudo apt install libsqlite3-dev:amd64
```

Now download the `sqlite_wrapper.c` file from this repository, transfer it to the VM, and paste the following in the terminal:
```bash
x86_64-linux-gnu-gcc -shared -fPIC -o libccl_sqlite3.so sqlite_wrapper.c -I/usr/include -L/usr/lib/x86_64-linux-gnu -lsqlite3
cp libccl_sqlite3.so ~/intelFPGA_lite/18.1/quartus/linux64/
```
## Environment Variables

To ensure binaries are located and executed under the correct architecture, the following variables were added to the end of the `~/.bashrc` file.

Open the `~/.bashrc` file with a text editor (such as `nano`):
```bash
nano ~/.bashrc
```

Paste the following lines at the end of the file:

```bash
export QSYS_ROOTDIR="/home/YOUR_USERNAME/intelFPGA_lite/18.1/quartus/sopc_builder/bin"
export QENV_DISABLE_AVX=1
export MALLOC_CHECK_=0
export PATH=$PATH:/home/YOUR_USERNAME/intelFPGA_lite/18.1/quartus/bin
export MTI_VCO_MODE=32
export PATH=/home/YOUR_USERNAME/intelFPGA_lite/18.1/modelsim_ase/bin:$PATH
```
Press `control + O`, `return` and `control + X` to save and exit.

After adding them, run:
```bash
source ~/.bashrc
```

## Quartus Environment Script Fix (`qenv.sh`)
To allow the startup script to run inside the ARM64 VM without aborting, we modify the architecture detection rules in the `qenv.sh` file.

0. **Grant write permission and open the file:**
```bash
chmod +w ~/intelFPGA_lite/18.1/quartus/adm/qenv.sh
nano ~/intelFPGA_lite/18.1/quartus/adm/qenv.sh
```
1. **Inject ARM64 architecture detection:**
Locate the line containing `# We don't support processors without SSE extensions` and insert the following code block right above it:
```bash
if test `uname -m` = "aarch64" ; then
    export QUARTUS_BIT_TYPE=64
fi
```
2. **Bypass x86 processor validation:**
Comment out (add `#` to the beginning of the lines) the entire block of code located between the line:

`# We don't support processors without SSE extensions...`

And the line:

`##### Determine what bitness executables...`

3. **Save and exit:**
Press `control + O`, `return` and `control + X` to save and exit.

## ModelSim Compatibility Patch
ModelSim (ASE) uses an internal script called `vco` to manage the runtime environment and Linux kernel detection. On modern distributions (and under ARM/emulation environments), this script fails when attempting to locate 32-bit binaries. To make the script editable and apply the necessary fixes:

```bash
chmod +w ~/intelFPGA_lite/18.1/modelsim_ase/vco
nano ~/intelFPGA_lite/18.1/modelsim_ase/vco
```
### `vco` File Modifications Table

| Line / Snippet | Original Code | Modified Code | Editing Location |
| :---: | :---: | :---: | :---: |
| **Architecture Definition** | `mode=${MTI_VCO_MODE:-""}` | `mode=${MTI_VCO_MODE:-"32"}` | <img width="721" height="452" alt="GNU nano 5 4" src="https://github.com/user-attachments/assets/3f076f04-d886-4717-87fe-ff508b1176dc" /> |
| **Library Injection** | *(Bellow `` dir=`dirname "$arg0"` ``)* | `export LD_LIBRARY_PATH=${dir}/lib32:$LD_LIBRARY_PATH` | <img width="722" height="456" alt="File Edit View Terminal Tabs Heig" src="https://github.com/user-attachments/assets/b6bad7cc-8e68-456e-b8c6-77ec6d420c30" /> |
| **Kernel Handling** | `*) vco="linux_rh60" ;;` | `*) vco="linux" ;;` | <img width="723" height="456" alt="Terminal - raphael-debian" src="https://github.com/user-attachments/assets/31567467-7fe0-42e8-bbca-d709327fe1b7" /> |

Press `control + O`, `return` and `control + X` to save and exit.

## ModelSim Graphical Interface Fix (FreeType)
ModelSim requires a specific legacy version of FreeType. It was necessary to compile version `2.4.12` from source using 32-bit flags. First, install GCC and build tools for 32-bit:
```bash
sudo apt install -y gcc:i386 g++:i386 make:i386 curl tar bzip2
```

Now download the original FreeType 2.4.12 source code and compile it targeting the i686 architecture:
```bash
cd ~/Downloads
curl -L -o freetype-2.4.12.tar.bz2 "https://downloads.sourceforge.net/project/freetype/freetype2/2.4.12/freetype-2.4.12.tar.bz2"
tar -xjvf freetype-2.4.12.tar.bz2
cd freetype-2.4.12
./configure --build=i686-pc-linux-gnu CC="gcc" "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
make -j$(nproc)
```

Next, remove any previously created lib32 directory to avoid permission conflicts, create ModelSim's dedicated folder, and copy the generated files:
```bash
sudo rm -rf ~/intelFPGA_lite/18.1/modelsim_ase/lib32
mkdir -p ~/intelFPGA_lite/18.1/modelsim_ase/lib32
cp objs/.libs/libfreetype.so.6* ~/intelFPGA_lite/18.1/modelsim_ase/lib32/
ln -sf /usr/lib/i386-linux-gnu/libfontconfig.so.1 ~/intelFPGA_lite/18.1/modelsim_ase/lib32/
sudo chown -R $USER:$USER ~/intelFPGA_lite/18.1/modelsim_ase/lib32
```

Finally, force ModelSim to load the new lib32 directory before system libraries by adding the variable to your `~/.bashrc`:
```bash
echo 'export LD_LIBRARY_PATH=$HOME/intelFPGA_lite/18.1/modelsim_ase/lib32:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

All set! With these fixes applied, the environment is fully configured and ready for you to compile your projects in Quartus Prime and run simulations in ModelSim without architecture errors or segmentation faults.

## Useful Commands
Now you can run Quartus workflows via terminal (CLI):

* **Compile** (Synthesize, Map, and Fit) a project:
```bash
quartus_map --read_settings_files=on project_name
quartus_fit --read_settings_files=on project_name
quartus_asm --read_settings_files=on project_name
```

This yields the `.sof` file to flash onto the board using `openFPGAloader`.

* **Simulate** using ModelSim (VHDL):
```bash
vlib work
vcom project.vhd 
vcom tb_project.vhd 
vsim tb_project -do "add wave -radix binary *; run 100 ns; wave zoom full; quit"
```

<table border="0" style="width: 100%;">
  <tr>
    <td width="50%" align="center">
      <img src="https://github.com/user-attachments/assets/223815f8-3745-42de-b537-338510eb7e32" width="100%" alt="PinPlanner" />
    </td>
    <td width="50%" align="center">
      <img src="https://github.com/user-attachments/assets/267614ee-cfde-46c9-8c07-5ec35b1dff85" width="100%" alt="ModelSim" />
    </td>
  </tr>
</table>

### Regarding the Intel Quartus Prime Lite Edition License
**Quartus Prime Lite Edition** is distributed for free by Intel/Altera, with an important caveat: the free license covers only a specific list of supported devices—entry-level families such as **Cyclone**, **MAX 10**, and **Arria II**, among others. Medium and high-performance devices (such as the **Arria 10**, **Stratix**, and **Cyclone V GX/GT** families in certain configurations) require a paid license from the **Standard** or **Pro** editions.

> This repository documents only the **virtualization and execution** process of the tool on Apple Silicon. The use of Quartus Prime is subject to Intel/Altera's own licensing terms, available on their official website.

<img width="1470" height="956" alt="TelaCheiaQuartusMac" src="https://github.com/user-attachments/assets/d6fbd427-ee68-4bec-8fdc-0c9b28e79a7d" />
