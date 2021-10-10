# Twisted Metal 4 Parser (.MR parser)
Parser for MR files. There are 2 dedicated programs for .MR files as release:
1. mr-unzip - "unzips" all content inside .MR files. You might find some interesting info inside such as sounds.
2. mr-to-obj - converts ".MR" file into wavefront .obj format.

# Download
To download these programs please visit [the releases section](https://github.com/charlieamer/TwistedMetal4Parser/releases).

# Extracting ("unzipping") .MR file content
You can extract all info inside .MR files using `mr-unzip.exe` program. Simply drag your .MR file into this program and you will find next to your MR file a folder named `YOUR_MR_FILE.extracted` and you can take a look what is inside. You can find sound/audio files like this for example.

# Map weapon locations
In case you want location of powerups inside map, I've created a blender importer addon. To obtain it follow the following steps:

1. Visit [this link](https://raw.githubusercontent.com/charlieamer/TwistedMetal4Parser/main/blender-tools/tm4_weapon_location_importer.py)
2. Download the file by click CTRL + S (or CMD + S if you are on mac).
3. In your blender go to `Edit > Preferences > Add-ons` and click `Install...` button in upper right corner.
4. Navigate to file you downloaded earler. It should be in your "Downloads"  with name `tm4_weapon_location_importer.py`
5. Convert map .MR file using latest version of `mr-to-obj` program. You can obtain it in [the releases section](https://github.com/charlieamer/TwistedMetal4Parser/releases)
6. Go to `File > Import > Import TM4 weapon locations`
7. Navigate to where the .MR file was converted to .obj and find `YOUR_MAP_NAME.txt` file and click it.