# James Butcher
# original: 5/18/22
# version: 2 - only includes setup.exe and image files now
#
# This program creates a file "filenames_index.txt": a helper file that contains a list
# of all the filenames to be downloaded from the auto-updater GitHub
# repository when the auto-update procedure is called.
# It grabs all the files found in the "Output" folder, removes the preceding "Output\"
# string from the file path, removes any files that shouldn't be included (the .git folder, for example)
# and writes each file name to its own line.

from pathlib import Path

url_filename = 'filenames_index.txt'
p = Path("Output")

print("Generating filenames index file...")

filenames = ["setup.exe", "Laser Controller.msi"]

filenames_string = '\n'.join(filenames)

with open(p / url_filename, 'w') as urls_file:
    urls_file.write(filenames_string)

print("Finished.")
