# Twisted Metal 4 Parser (.MR parser)
Parser for MR files.


![yard-render](https://user-images.githubusercontent.com/5943275/109431818-ac102b80-7a08-11eb-8087-d033d0efda43.png)


![tm cars](https://user-images.githubusercontent.com/5943275/109431837-baf6de00-7a08-11eb-89a0-11619bdeccd9.png)


There are 2 dedicated programs for .MR files as release:
1. mr-unzip - "unzips" all content inside .MR files. You might find some interesting info inside such as sounds.
2. mr-to-obj - converts ".MR" file into wavefront .obj format.

# Download
To download these programs please visit [the releases section](https://github.com/charlieamer/TwistedMetal4Parser/releases).

# Extracting ("unzipping") .MR file content
You can extract all info inside .MR files using `mr-unzip.exe` program. Simply drag your .MR file into this program and you will find next to your MR file a folder named `YOUR_MR_FILE.extracted` and you can take a look what is inside. You can find sound/audio files like this for example.

# Map weapon locations
In case you want location of powerups inside map, I've created a blender importer addon. To obtain it follow the following steps:

1. Download addon from [here](https://github.com/charlieamer/TwistedMetal4Parser/releases/download/v1.1/tm4_weapon_location_importer.py)
1. In your blender go to `Edit > Preferences > Add-ons` and click `Install...` button in upper right corner.
1. Navigate to file you downloaded earler. It should be in your "Downloads"  with name `tm4_weapon_location_importer.py`
1. Convert map .MR file using latest version of `mr-to-obj` program. You can obtain it in [the releases section](https://github.com/charlieamer/TwistedMetal4Parser/releases)
1. Go to `File > Import > Import TM4 weapon locations`
1. Navigate to where the .MR file was converted to .obj and find `YOUR_MAP_NAME.txt` file and click it.

# mr-to-obj .Exe usage:
If you decide to go through experience of converting map/cars yourself, here are the steps to do it:

1. Extract TM4 files in a folder, and remember where you extracted it.
1. Download mr-to-obj.exe (you can find it bellow this text), and remember where you saved it.
1. Go to a map/car file you want to open (found in DATA folder of step 1).
1. Right click the map/car (for example, YARD.MR), click "Open with".
1. Click More Apps -> Look for another app on this PC
1. Find the program that you downloaded at step 2.
1. You will find converted .OBJ file in the same folder as .MR file was found.
1. In order to view .OBJ file, you can use: Blender, Windows 3D viewer (just double click by default) or for example upload it here: https://www.creators3d.com/online-viewer

*NOTE: If you are trying to convert map, make sure there is .IMG file in the same folder as your map file.*

# Map conversion limitations
## Map details (teleports, etc)
This data is readable (meaning, it is possible to make a script that would output such values), but I didn't know how to meaningfully output such data to .obj map files. In case you need this data please contact me (contact info is below).

## Vertex colors in maps
This game's maps heavily relies on vertex coloring to give a bit of extra lighting feel. Even though this converter outputs vertex colors, not many viewers support that. It seems the program doesn't convert all the maps perfectly though, so beware of that.

### Solution
What you can do is use program "Mesh Viewer" to open .obj files and then convert files to other formats that support vertex colors. I think .dae might be the format that supports it.

## Missing textures / models in maps
Not all meshes are converted because there are some meshes that are created in runtime (for example, train in neon city, upper part of crane in construction yard, etc.). That's why when you open .png file of maps, you'll see a lot of texture is missing/transparent. Those are accessed through game code in runtime and at the moment this tool can't inspect such textures/models.

# Other utilities
## mr-unpackager.exe
This program "unzips" the .MR. Output folder is called `SAME_FILENAME.extracted`, and it will be located in the same folder as .MR file. For example, if you want to "unzip" `AXEL.MR`, it will be extracted to folder `AXEL.extracted`. **Existing files will be overwritten**.

You can then edit or examine files if you want to.

*Please keep all folders inside named `NODE_...`, because there are instances where a file and folder have same name, and this would create conflicts.*

Usage: `mr-unpackager FILENAME.MR`

## mr-packager.exe
This program "zips" a folder into a .MR file. Output file will be called `SAME_FILENAME.MR`, and it will be located in the same folder as the input folder. **You MUST choose a folder that ends with .extracted**. For example, if you want to convert folder `AXEL.extracted`, it will be converted to file called `AXEL.MR`. **Existing .MR file will be overwritten**.

If you modified something in folders, you can put back the modified .MR file back to CD image (or .iso file) and you'll have the modded game!!!

*Please keep all folders inside named `NODE_...`, because there are instances where a file and folder have same name, and this would create conflicts.*

Usage: `mr-packager FOLDERNAME.extracted`

## mr-diff.exe
This program tells you if two .MR files have same content. This actually goes through each node and component and checks if they are the same. **Only the frist found difference is shown in output**.

Usage: `mr-diff FILE1.MR FILE2.MR`

## palette-to-png.exe and png-to-palette.exe (Editing palettes/colors in textures)
Before using these 2 programs, first unpackage .IMG file. Inside you need to find .clt--12 file. This is the palette.

* `palette-to-png`: provide path to a file which ends with .clt--12 and it will convert it to png. The file will be saved like this: palette__originalname.clt--12__offsetx__offsety. It is important that you don't rename the png file.
* `png-to-palette`: provide path to png file and it will convert it to .clt--12 file.
The flow to edit palette works like this:
```
mr-unpackager PATH_TO.IMG
palette-to-png.exe PATH_TO_CLT
edit the png with gimp/photoshop/whatever
png-to-palette.exe PATH_TO_PNG
mr-packager PATH_TO.IMG.extracted
Re-package iso/whatever is your flow
```
Enjoy

# Contact
In case you ever need help with this program, contact me:
Reddit: https://www.reddit.com/user/Guilty_Painter_4707
Discord: charlieamer#9302
