<p align="center">
	<h1 align="center"><b>Terminal Media Viewer</b></h1>
	<p align="center"><b>View images and videos without leaving the console</b></p>
	<p align="center">
	<img src="https://img.shields.io/github/license/kal39/TerminalMediaViewer">
	<img src="https://img.shields.io/github/issues/kal39/TerminalMediaViewer">
	<br>
	<img src="https://img.shields.io/github/repo-size/kal39/TerminalMediaViewer">
	<img src="https://img.shields.io/github/languages/top/kal39/TerminalMediaViewer">
	<!--- <img src="https://img.shields.io/github/v/release/kal39/TerminalMediaViewer"> --->
	</p>
</p>

----

### Features

* View images form the terminal
* Watch videos from the terminal (with sound)
* Play videos at any fps
* Resize images / videos (aspect ratio can be changesd or preserved)
* Easy to use

----

### Screenshots

-- WIP --

----

### Requirements

A terminal that supports **truecolor** ([list](https://gist.github.com/XVilka/8346728)) and **utf-8** (most terminals should support utf-8).

----

### Usage

**tmv [`OPTIONS...`] `INPUT`**

* **`INPUT`**  
	File to display/play

* **`OPTIONS...`**  
	* `-f`, `--fps`  
		Set fps (default 15 fps)
	* `-F`, `--origfps`  
		Use original fps from video (default 15 fps)
	* `-h`, `--height`  
		Set height (setting both `width` and `height` will ignore original aspect ratio)
	* `-w`, `--width`  
		Set width (setting both `width` and `height` will ignore original aspect ratio)
	* `-?`, `--help `  
		Display help


----

### Binaries

-- WIP --

----

### Build from source

**Requirements:**
* `gcc`
* `make`
* `lavcodec`
* `ffmpeg`
* `libavcodec`
* `libavformat`
* `libavfilter`
* `libavdevice`

```
git clone https://github.com/kal39/TerminalMediaViewer.git
cd TerminalMediaViewer
make
```

----

### Contributing
Any contributions are greatly appreciated.

----

**kal39**(https://github.com/kal39) - kal390983@gmail.com  
Distributed under the MIT license. See `LICENSE` for more information.
