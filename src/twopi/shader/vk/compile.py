import os
import glob

if __name__ == "__main__":
  filenames = glob.glob('*.vert') + glob.glob('*.frag')
  for filename in filenames:
    print(f'compiling {filename}:')
    os.system(f'glslc.exe {filename} -o {filename}.spv')
