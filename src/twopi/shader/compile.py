import os
import glob

if __name__ == "__main__":
  filenames = glob.glob('*.vert') + glob.glob('*.frag')
  for filename in filenames:
    print(f'compiling {filename}:')
    if os.system(f'glslc.exe {filename} -o {filename}.spv') != 0:
      # delete previously compiled spv file
      if os.path.exists(f'{filename}.spv'):
        os.remove(f'{filename}.spv')
