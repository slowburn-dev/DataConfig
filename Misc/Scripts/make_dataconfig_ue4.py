import sys
assert sys.version_info.major == 3 and sys.version_info.minor >= 4, "needs python >= 3.4"

import os
from pathlib import Path
import shutil

def _copy(src_path, dest_path):
    shutil.copy(str(src_path), str(dest_path))

def _copytree(src_path, dest_path):
    shutil.copytree(str(src_path), str(dest_path), dirs_exist_ok=True)

root = Path(os.path.abspath(__file__)).parent.parent.parent
plugin = root / "Tmp/UE4/DataConfig"
plugin.mkdir(parents=True, exist_ok=True)

# copy uplugin
root_uplugin = root / "DataConfig/DataConfig4.uplugin"
plugin_uplugin = plugin / "DataConfig.uplugin"
_copy(root_uplugin, plugin_uplugin)

# copy sources
root_src = root / "DataConfig/Source"
plugin_src = plugin / "Source"
plugin_src.mkdir(parents=True, exist_ok=True)

_copytree(root_src / "DataConfigCore", plugin_src / "DataConfigCore")
_copytree(root_src / "DataConfigExtra", plugin_src / "DataConfigExtra")
_copytree(root_src / "DataConfigTests", plugin_src / "DataConfigTests")
_copytree(root_src / "DataConfigEditorExtra", plugin_src / "DataConfigEditorExtra")

# copy others
_copytree(root / "DataConfig/Content", plugin / "Content")
_copytree(root / "DataConfig/Tests", plugin / "Tests")

print("copied clean DataConfig UE4 to: %s" % plugin)
os.startfile(str(plugin.parent))
