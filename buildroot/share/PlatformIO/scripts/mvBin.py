import pioutil
import marlin
import shutil
Import("env")
def mvBinary(source, target, env) :
  print("Moving Bin...");
  shutil.copy2(target[0].path, target[0].dir.path + '/../../../binaries');

marlin.add_post_action(mvBinary)
