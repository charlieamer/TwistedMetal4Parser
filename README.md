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
2. Download the file by click CTRL + S (or CMD + S if you are on mac).
3. In your blender go to `Edit > Preferences > Add-ons` and click `Install...` button in upper right corner.
4. Navigate to file you downloaded earler. It should be in your "Downloads"  with name `tm4_weapon_location_importer.py`
5. Convert map .MR file using latest version of `mr-to-obj` program. You can obtain it in [the releases section](https://github.com/charlieamer/TwistedMetal4Parser/releases)
6. Go to `File > Import > Import TM4 weapon locations`
7. Navigate to where the .MR file was converted to .obj and find `YOUR_MAP_NAME.txt` file and click it.

# mr-to-obj .Exe usage:
If you decide to go through experience of converting maps yourself, here are the steps to do it:

1. Extract TM4 files in a folder, and remember where you extracted it.
1. Download mr-to-obj.exe (you can find it bellow this text), and remember where you saved it.
1. Go to a map/care file you want to open (found in DATA folder of step 1).
1. Right click the map/car (for example, YARD.MR), click "Open with".
1. Click More Apps -> Look for another app on this PC
1. Find the program that you downloaded at step 2.
1. You will find converted .OBJ file in the same folder as .MR file was found.
1. In order to view .OBJ file, you can use: Blender, Windows 3D viewer (just double click by default) or for example upload it here: https://www.creators3d.com/online-viewer


# Map conversion limitations
## Map details (teleports, etc)
This data is readable (meaning, it is possible to make a script that would output such values), but I didn't know how to meaningfully output such data to .obj map files. In case you need this data please contact me (contact info is below).

## Vertex colors in maps
This game's maps heavily relies on vertex coloring to give a bit of extra lighting feel. Even though this converter outputs vertex colors, not many viewers support that. It seems the program doesn't convert all the maps perfectly though, so beware of that.

### Solution
What you can do is use program "Mesh Viewer" to open .obj files and then convert files to other formats that support vertex colors. I think .dae might be the format that supports it.

## Missing textures / models in maps
Not all meshes are converted because there are some meshes that are created in runtime (for example, train in neon city, upper part of crane in construction yard, etc.). That's why when you open .png file of maps, you'll see a lot of texture is missing/transparent. Those are accessed through game code in runtime and at the moment this tool can't inspect such textures/models.

# Contact
In case you ever need help with this program, contact me:
Reddit: https://www.reddit.com/user/Guilty_Painter_4707
Discord: charlieamer#9302