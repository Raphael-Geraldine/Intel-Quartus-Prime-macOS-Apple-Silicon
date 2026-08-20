# Relevant information about programming on macOS for DE10-Lite boards

> As of now, the latest version of `openFPGAloader` only accepts a few file types, such as `.pof` and `.svf`; however, traditional `.sof` files have not yet been decoded by `openFPGAloader` for the DE10-Lite, because
> they contain certain proprietary Intel/Altera patterns and metadata.

To replicate the effect of `.sof` files, which are volatile and saved exclusively to SRAM without erasing Flash memory, `.svf` files can be used with the modifications highlighted below:

## Installing openFPGAloader

First, install and unzip the `.zip` file from the [Releases](https://github.com/Raphael-Geraldine/Intel-Quartus-Prime-macOS-Apple-Silicon/releases/tag/DE10-Lite) tab. It contains a
modified version of this tool's source code (keeping the terms of the original license). After that, paste the scripts below into the terminal, which will **remove the current active version** of `openFPGAloader` from your macOS (if it exists) and install the necessary dependencies:

```zsh
brew uninstall -f openfpgaloader
brew install --only-dependencies openfpgaloader
brew install cmake pkg-config zlib
```

Now, go into the macOS folder where the `.zip` was unzipped (adjust the name if needed):

```zsh
cd ~/Downloads/openFPGALoader
```

And compile and install the new version, on macOS:

```zsh
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
sudo make install
```

## How to generate an `.svf` file

1st) After having a project ready and compiled into `.sof`, **in Quartus**, go to "Tools > Programmer > Add file..." and add the `.sof` file that should go to the board;

2nd) Go to "File > Create JAM, JBC, SVF or ISC File…"; choose `.svf` and change the frequency to 24.0 MHz;

3rd) Just click "OK" and transfer the file to macOS through the shared folder.

<p align="center">
  <img width="707" height="555" alt="svf" src="https://github.com/user-attachments/assets/f693064e-027c-4369-9d81-9dc1e9941c8e" />
</p>

## Important modification to the file

By default, `.svf` files contain instructions to erase Flash memory when uploaded
to the board. To run the circuit directly on SRAM and keep the Flash intact (exactly like `.sof` does):

1st) Open the `.svf` file in TextEdit.

2nd) Delete the block that starts at `!Max 10 DSM Clear` and goes until the end of `!Max 10 DSM Verify` (right before `!Max 10 Program ICB`; it should look similar to the image below).

3rd) Save and upload the `.svf` normally.

<p align="center">
  <img width="768" height="534" alt="Screenshot 2026-08-20 at 17 50 07" src="https://github.com/user-attachments/assets/5b40a98e-6594-40a0-bc1c-7e7d15c25267" />
</p>

## Uploading the `.svf` to the DE10-Lite

Just paste/type this into the macOS terminal:

```zsh
openFPGALoader -c usb-blaster -m path/to/file.svf
```

### What to do if you forget the modification

In this scenario, the file contained in Flash memory will have been erased. If you want to return to the default factory file, you can download it from the official [Terasic](https://download.terasic.com/downloads/cd-rom/de10-lite/DE10-Lite_v.2.2.0_SystemCD.zip) website.
The `.pof` file to be uploaded again has the following path: `"DE10-Lite_v.2.2.0_SystemCD/Demonstrations/Default/DE10_LITE_Default.pof"`. To upload it to the board, just paste/type this into the macOS terminal:

```zsh
openFPGALoader -c usb-blaster -f path/to/DE10_LITE_Default.pof
```
