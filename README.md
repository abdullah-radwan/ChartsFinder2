# Charts Finder 2.3
Charts Finder is an app to get IFR charts for your flights.

## Installation
Go to releases and download the latest release. The pre-compiled versions are provided for Windows and Linux.

## Build
You need to have Qt 5, [QtAutoUpdater](https://github.com/Skycoder42/QtAutoUpdater), QGumboParser, and cURL installed. 
Create a folder named 'lib' and put the compiled library files in it. Create a folder named 'include' inside it and put the include files in it.

You'll need to create 'translations' folder in the executable folder. Compile Charts Finder 2 .ts files, and copy Qt and QtAutoUpdater .qm files into it to have the translations.

### Windows:
Assuming Qt 5 binaries are in PATH, and using mingw32:

```
git clone https://github.com/abdullah-radwan/ChartsFinder2.git ChartsFinder2
cd ChartsFinder2
qmake
mingw32-make
mingw32-make clean
```

### Linux:

```
git clone https://github.com/abdullah-radwan/ChartsFinder2.git ~/ChartsFinder2
cd ~/ChartsFinder2
qmake
make
make clean
chmod u+x ChartsFinder2
./ChartsFinder2
```

## Credits
Copyright © [Material Design Icons](https://github.com/Templarian/MaterialDesign) under SIL Open Font 1.1 license.

Copyright © [Sergey Lagner](https://github.com/lagner/QGumboParser) under MIT license.

## About
The program is based on C++, Qt 5, QtAutoUpdater, QGumboParser, and cURL. It's under GPL v3 license.

Any suggestions and contributes are welcomed.

Copyright © Abdullah Radwan
