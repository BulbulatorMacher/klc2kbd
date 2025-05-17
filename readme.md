# KLC2KBD
Converts `MSKLC` keyboard layouts from to `KBD` used by Windows 95/98/Me.
## Introduction
Microsoft released a tool for creating custom keyboard leayouts called `Microsoft Keyboard Layout Creator (MSKLC)`.
`MSKLC` supports Windows 2000 and newer (Xp, Vista, 7, 8, 10). Windows 95, 98 and Me use different format and are not supported.
There are third party keyboard layout editors for these older systems but are no longer supported.
Furthermore only basic functionality is free, more advanced features require paid version which is currently hard to obtain.

Instead of creating another layout editor from scratch I decided to make a utility to convert `MSKLC` layout to older Windows versions.
Almost all of the features are supported: standard, AltGr, dead keys, ligatures, caps lock etc... It works for layouts for various languages.
There are slight differences how old vs new Windows versions behave but majority should be working correctly.

Although `klc2kbd` can create a file usable in Windows 95/98/Me there are some manual steps required to install it properly on the Windows 95/98/Me.
User needs a newer system (preferably Windows 10/11) to use `MSKLC` and build and run `klc2kbd` before copying resulting file to Windows 95/98/Me.

Manual steps require modification of a registry. BAT script using `REG` command would be obvious choice but this command is not present on Windows 95/98/Me.
Another approach could be using `C` with `WinApi` to access registry but this would require creating another application and building it on a Windows 95/98/Me.
## Building the source code
Source code uses standard C++11 so a build can be done in a typical fashion, for example `g++ -std=c++11 *.cpp -o klc2kbd`
## Using application to generate `KBD` file
- Following steps in this chapter will be done on the moder system (Windows 10/11)
- `MSKLC` can be downloaded from: https://www.microsoft.com/en-us/download/details.aspx?id=102134
- Create layout in `MSKLC`. Remember to set correct language in Project Properties.
There is no need to build Dll/Setup as `klc2kbd` uses only `KLC` file which is used to save the project.
- (optionally) correct `LOCALEID` in the `KLC` file as described in the next chapters
- Windows 98 uses codepages and can only support couple of languages at once. 
Make sure Windows 98 language, keyboard layout language and codepage are all compatible with each other.
Download codepage corresponding to the selected language. 
It can be downloaded from: https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/ \
Windows-1252 (English, French, German, Portugese, Spanish and more) is default and can be omitted
- Call `klc2kbd` with the following parameters: `kbc2kbd --cp "path_to_codepage.TXT" -o "path_to_generated.KBD" "path_to_source.KLC"`\
for example `kbc2kbd --cp CP1250.TXT -o Kbdpl2.kbd polski_custom.klc`\
(for Windows-1252 `--cp "path_to_codepage.TXT"` can be omitted)
## Installing `KBD` file in Windows 95/98/Me
### Determining `LOCALEID` value
Field `LOCALEID` has a form of `xxxxyyyy` in a hex format, where `yyyy` is a language code and `xxxx` starts from `0000` and is incremented for each layout for a given language.
To find a suitable value for the new layout on **Windows 95/98/Me** machine open regedit and locate `HKEY_LOCAL_MACHINE\system\currentcontrolset\control\keyboard layouts`.
You will find there keys named in the `xxxxyyyy` format. Find a next free value for the language.
For demonstration I will use Polish which has a value of `xxxx0415`.
Standard installation has 2 layouts for Polish: `00000415` and `00010415`.
In this example the next free value would be `00020415`
### Correcting `LOCALEID` in `KBD` file
`yyyy` is taken from the `KLC` file and thus only `xxxx` part needs to be changed.
It can be done in one of two ways: editing `KLC` in text editor **BEFORE** generating `KBD` or editing generated `KBD` in a hex editor.
- Open `KLC` file in a text editor, locate the `LOCALEID` line.
It will likely have a value of `"0000yyyy"`.
Change it to the value determined in a previous step.
Continuing Polish example change it from `"00000415"` to `"00020415"`
Save the `KLC` file, run the application to convert it to `KBD`

**OR**
- Open `KBD` file in a hex editor. `LOCALEID` occupies 4 bytes in Little-Endian format on positions 02 to 05 (counting from 00).
Note that in this format value looks reversed: `15 04 00 00`
Bytes 02 and 03 are a language code `yyyy` while bytes 04 (low byte) and 05 (high byte) are `xxxx`.
In Polish example byte 04 needs to be changed to `02` and byte 05 will be left at `00` resulting in `15 04 02 00`.
### Copying `KBD` file to correct location
Rename the `KBD` file and place int in the `C:\WINDOWS\SYSTEM\` folder on the **Windows 95/98/Me** pc.
`KBD` files are usually named `Kbd` + `language code` + `number` + `.kbd`.
In this example file should be named `Kbdpl2.kbd`
### Determining `layout id` value
Open regedit on **Windows 95/98/Me**, in the `HKEY_LOCAL_MACHINE\system\currentcontrolset\control\keyboard layouts`
check all of the `xxxxyyyy` keys and inspect all of the `layout id` values contained in them (they are in a hex format).
Choose a next free value for `layout id`
### Creating registry entries
- Open regedit on **Windows 95/98/Me**, in `HKEY_LOCAL_MACHINE\system\currentcontrolset\control\keyboard layouts` create a key corresponding to LOCALEID `xxxxyyyy` (for exaple `00020415`).
- In that key crate string value named `layout file` with value correspondig to `KBD` filename (like `Kbdpl2.kbd`)
- In that key create another string value named `layout text`. This will be displayed when selecting layout (for example `Polski (custom)`)
- Yet another string value named `layout id` with corresponding value determined earlier
### Selecting the new layout
- Restart the **Windows 95/98/Me** for the new layout to be visible
- Select `Start`, `Settings`, `Control panel`, `Keyboard`
- Add or select `Language`
- In language properties select the new layout
## Testing layout
- Use `MSKLC` `Project` `Test Keyboard Layout` to test layout
- On Windows 95/98/Me you can use Notepad to test layout
- On Windows Me `On-Screen Keyboard` can be used
## Resources
Informations necessary to develop `klc2kbd` came from `Microsoft Windows DDK.iso` which can be downloaded here: https://archive.org/details/tool-microsoft-ddk-wdk \
Document describing keyboard layout files is located on CD under path: `DOCS\DESGUIDE\KEYCNT.DOC`\
Keyboard layout files in x86 assembly are located on CD under path: `KEYB\SAMPLES\DRIVERS\LAYOUT\`\
Document does provide some of the info about KBD file format but not all. Remaining has been reverse-engineered based on the contents of the ASM files.