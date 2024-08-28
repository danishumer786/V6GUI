
import sys

def replaceTabsWithFourSpaces(filename):

    contents = []
    with open(filename, "r") as original:
        contents = original.readlines()

    with open(filename, "w") as modified:
        for line in contents:
            modified_line = line.replace('\t', '    ')
            modified.write(modified_line)


if __name__ == "__main__":

    for arg in sys.argv:
        replaceTabsWithFourSpaces(arg)
        
