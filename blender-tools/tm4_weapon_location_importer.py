bl_info = {
    "name": "Import Twisted Metal 4 weapon locations",
    "blender": (2, 80, 0),
    "category": "Import-Export",
    "location": "File > Import > Import TM4 weapon locations",
}

import bpy
import math

def read_location_data(context, filepath):
    f = open(filepath, 'r', encoding='utf-8')

    bpy.ops.object.select_all(action='DESELECT')

    added_objs = []
    for line in f.readlines():
        (node_name, weapon_name, x, y, z) = line.split(" ")
        bpy.ops.object.empty_add(type='CONE', align='WORLD', location=(float(x), -float(z), float(y)), scale=(1, 1, 1), rotation=(math.pi / 2.0, 0, 0))
        bpy.context.object.name = weapon_name + '_' + node_name
        added_objs.append(bpy.context.object)

    f.close()
    
    for obj in added_objs:
        obj.select_set(True)

    return {'FINISHED'}

from bpy_extras.io_utils import ImportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator

class ImportTM4WeaponLocations(Operator, ImportHelper):
    """This appears in the tooltip of the operator and in the generated docs"""
    bl_idname = "twisted_metal_4_importer.weapon_locations"
    bl_label = "Import TM4 weapon locations"

    # ImportHelper mixin class uses this
    filename_ext = ".txt"

    filter_glob: StringProperty(
        default="*.txt",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    def execute(self, context):
        return read_location_data(context, self.filepath)


# Only needed if you want to add into a dynamic menu
def menu_func_import(self, context):
    self.layout.operator(ImportTM4WeaponLocations.bl_idname, text="Import TM4 weapon locations")


def register():
    bpy.utils.register_class(ImportTM4WeaponLocations)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)


def unregister():
    bpy.utils.unregister_class(ImportTM4WeaponLocations)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)


if __name__ == "__main__":
    register()

