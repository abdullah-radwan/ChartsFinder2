# Charts Finder 2.1
Charts Finder is a tool to get charts for your flights.

Read the manual for full explanation.

## Installation
Go to releases and download the latest release. The pre-compiled versions are provided for Windows and Linux.

## Build
You need to have Qt 5, [QtAutoUpdater](https://github.com/Skycoder42/QtAutoUpdater), and cURL installed for Linux.
On Windows, the library is included precompiled.

```
git clone https://github.com/abdullah-radwan/ChartsFinder2.git ~/ChartsFinder2
cd ~/ChartsFinder2
qmake
make
make clean
cd src
chmod u+x ChartsFinder2
./ChartsFinder2
```

You'll need to create 'translations' folder in the executable folder, and copy Qt, QtAutoUpdater and Charts Finder 2 .qm files into it in order to make the translations work.

If you want to build the installer, you need to have Qt Installer Framework installed.

To build the installer, move the binary to installer/packages/abdullahradwan.chartsfinder2.1/data folder. On Linux, you'll need also to move icon.svg file exists in res/icons folder.
Then make a 7zip archive and remove other files.

Then run the following command in 'installer' folder, on Windows: `<path to installer framework>\binarycreator.exe -c config/config.xml -p packages installer.exe`.

On Linux, you need to edit 'installer/config/config.xml' file. Disable the Windows repository by setting enabled to 0, and enable Linux repository by setting enabled to 1.
Then run the following command: `binarycreator -c config/config.xml -p packages installer`.

## Credits
Copyright © [Material Design Icons](https://github.com/Templarian/MaterialDesign) under SIL Open Font 1.1 license.
Copyright © [Sergey Lagner](https://github.com/lagner/QGumboParser) under MIT license.

## About
The program is based on C++, Qt 5, QtAutoUpdater, QGumboParser, and cURL. It's under GPL 3 license.

Any suggestions and contributes are welcomed.

Copyright © Abdullah Radwan
