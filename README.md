<p align="center">
	<h1 align="center"><b>Terminal Media Viewer</b></h1>
	<p align="center"><b>View images and videos without leaving the console</b></p>
	<p align="center">
	<img src="https://img.shields.io/github/license/kal39/TerminalMediaViewer">
	<img src="https://img.shields.io/github/repo-size/kal39/TerminalMediaViewer">
	<img src="https://img.shields.io/github/languages/top/kal39/TerminalMediaViewer">
	<br>
	<img src="https://img.shields.io/github/issues/kal39/TerminalMediaViewer">
	<img src="https://img.shields.io/github/v/release/kal39/TerminalMediaViewer?sort=semver">
	</p>
</p>

----

### Features

* View **images** form any terminal
* Watch **videos** from any terminal
* Watch **youtube** videos from any terminal (`-y`, `--youtube`)
* Play videos at **any fps** (`-f`, `--fps`, `-F`, `--origfps`)
* **Resize** images / videos (`-w`, `-h`, `--width`, `--height`)
* Easy to use

----

### Examples

**Viewing an image**

<img src="assets/iGif.gif" width="80%">

**Watching a video**

<img src="assets/vGif.gif" width="80%">

----

### Requirements

* A terminal that supports **truecolor** ([list](https://gist.github.com/XVilka/8346728)) and **utf-8** (most terminals should support utf-8).
* [ffmpeg](https://github.com/FFmpeg/FFmpeg) (only for videos)
* [youtube-dl](https://github.com/ytdl-org/youtube-dl) (only for youtube videos)

----

### Usage

**tmv [`OPTIONS...`] `[INPUT FILE / URL]`**

* **`INPUT`**  
	File to display/play

* **`OPTIONS...`**  
	* `-y`. `--youtube`  
		View youtube videos
	* `-h`, `--height`  
		Set height (setting both `width` and `height` will ignore original aspect ratio)
	* `-w`, `--width`  
		Set width (setting both `width` and `height` will ignore original aspect ratio)
	* `-f`, `--fps`  
		Set fps (default 15 fps)
	* `-F`, `--origfps`  
		Use original fps from video (default 15 fps)
	* `-s`, `--no-sound`   
		Disable sound
	* `-i`, `--no-info`   
		Disable progress bar for videos
	* `-?`, `--help `  
		Display help
	* `-V`  
		Display version

----

### Installation

#### Linux

**Instructions:**
1. Install dependencies.  
	* `libavcodec-dev`
	* `libavformat-dev`
	* `libavfilter-dev`
	* `libavdevice-dev`

	In addition, to watch videos install:
	* `ffmpeg`
	* `youtube-dl`
2. Clone the repository and run make.
	```
	git clone https://github.com/kal39/TerminalMediaViewer.git
	cd TerminalMediaViewer
	make
	```
	To install tmv to `/usr/local/bin` you can run `make install` (needs sudo privileges).  
	To uninstall run `make uninstall` (needs sudo privileges).

#### MacOS

**Requirements:**
* `homebrew`
* `iterm2`

**Instructions:**  
1. Install dependencies
	```
	brew install argp-standalone
	```
	In addition, to watch videos:
	```
	brew install ffmpeg
	brew install youtube-dl
	```
2. Clone the repository and run make.
	```
	git clone https://github.com/kal39/TerminalMediaViewer.git
	cd TerminalMediaViewer
	make
	```
	To install tmv to `/usr/local/bin` you can run `make install` (needs sudo privileges).  
	To uninstall run `make uninstall` (needs sudo privileges).

> **Only works on iTerm2.**

> There are some performance issues. Depending on the video encoding, your mileage may vary.

----

### Releases

* **[`v0.1.1`](https://github.com/kal39/TerminalMediaViewer/releases/tag/v0.1.1) Youtube Support**  
	TerminalMediaViewer can now play videos directly from youtube.  
	To play videos from youtube, use the `-y` option.

	* Improved memory usage
	* Cursor is now hidden during videos
	* Supports spaces in video filenames
	* **Play videos directly from youtube**
	* Check if ffmpeg and YouTube exist before playing videos
	* Better error and debug messages

* **[`v0.1`](https://github.com/kal39/TerminalMediaViewer/releases/tag/v0.1) Initial release**  
	Initial release of tmv.  
	It is in a very early state so bugs are expected.

	* View images
	* Watch videos (with sound)
	* Resize images / videos

----

### Contributing
Any contributions are greatly appreciated.

----

**kal39**(https://github.com/kal39) - kal390983@gmail.com  
Distributed under the MIT license. See `LICENSE` for more information.
