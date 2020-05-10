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
	<img src="https://img.shields.io/github/workflow/status/kal39/TerminalMediaViewer/make/master">
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
	* `-?`, `--help `  
		Display help
	* `-V`  
		Display version

----

### Building from source
#### Linux
**Requirements:**
* `libavcodec-dev`
* `libavformat-dev`
* `libavfilter-dev`

**Instructions:**
1. Clone the repository.  
	`git clone https://github.com/kal39/TerminalMediaViewer.git`
2. `cd TerminalMediaViewer`
3. You can either build the the program locally with `make` or install the program to `/usr/local/bin/` with `make install`.  

To uninstall run `make uninstall`.

#### MacOS
**Requirements:**
* [`xcode`](https://apps.apple.com/us/app/xcode/id497799835)
* [`homebrew`](https://docs.brew.sh)
* `iterm2`
* `ffmpeg`
* `argp-standalone`
```
brew install iterm2
brew install argp-standalone
brew install ffmpeg
git clone https://github.com/kal39/TerminalMediaViewer.git
cd TerminalMediaViewer
make -f makefile.macos
```
Open iTerm2 to run tmv
Note: it's laggy. Depending on the video encoding, your mileage may vary.
Tested with iTerm 3.3.9, ffmpeg 4.2.2, MacOS Catalina 10.5.4, Xcode 11.4.1, argp-standalone 1.3

----

### Binaries

You can download pre-built binaries from the [**releases**](https://github.com/kal39/TerminalMediaViewer/releases) page.

----

### Contributing
Any contributions are greatly appreciated.

----

**kal39**(https://github.com/kal39) - kal390983@gmail.com  
Distributed under the MIT license. See `LICENSE` for more information.
