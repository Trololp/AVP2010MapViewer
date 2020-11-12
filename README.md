# AVP2010MapViewer

AVP2010MapViewer is map viewer for the game "Aliens versus Predator (2010)". Map viewer allow you to see a map itself, props, entities and provide additional information to it.

## Installation

Make sure your computer support DirectX 11. Download [last release v0.4](https://github.com/Trololp/AVP2010MapViewer/releases/tag/v0.4). unpack zip files into new folder. done.
## Development
I use **Visual Studio 2017**. Used **Microsoft DirectX SDK (June 2010)**. also you need **directXTK** for text on a screen (this was get from Nuget). There file in 
[usefull_stuff](https://github.com/Trololp/AVP2010MapViewer/tree/main/usefull_stuff) folder named **HACKS.lib** dont scare its is d3dcompiler v47 lib. with is solve
my problem with linking. find and delete in `AVP2010MapViewer.h` this stroke `#pragma comment(lib, "HACKS.lib")` if all compile alright.

## Usage

  ### Preparations
   1. Download [QuickBMS](https://aluigi.altervista.org/quickbms.htm)
   2. Then download script [asura.bms](https://github.com/Trololp/AVP2010MapViewer/blob/main/usefull_stuff/asura.bms)
   3. For maps you want to look at you need first unpack **.pc** files. Map Geometry located at **...\Aliens vs Predator\Envs\(Map_name)\**
   this is file called **(Map_name).pc** you need to unpack this into the first folder for continue. *Example: Jungle.pc located at \Aliens vs Predator\Envs\Jungle\ folder*
   Mission files located in same folder with map geometry and have specific prefix like. files like **A01_(Map_name).pc**. you need to unpack it in second folder.
   *Note : A - means alien mission M - marine P - predator.*
 
  ### Launching and controls
   **Launch**
   1. Select folder with map geometry. *Note: folder you extracted (Map_name).pc first*
   2. Select folder with mission. *Note: you can not chose any folder then you will be looking only at map geometry*
   3. **Wait**, it can take long to load all files needed.
   
   **Usage**
   use keys W, S, A, D to move camera. press RMB (Right mouse button) to turn camera around. press LMB on specific entity to select this.

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.
Please make sure to update tests as appropriate.
