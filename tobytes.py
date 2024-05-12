from PIL import Image
import math

def find_closest_color(rgb, palette):
    min_distance = float('inf')
    index_selected = 0
    for index, color in enumerate(palette):
        distance = math.sqrt(sum((c1 - c2) ** 2 for c1, c2 in zip(rgb, color)))
        if distance < min_distance:
            min_distance = distance
            index_selected = index
    return index_selected

file_name = input("Enter File Name: ")
im = Image.open(file_name)
pix = im.load()

print("Image Size: ", im.size)
print("Image Mode: ", im.mode)

Color_Plate = [
    (255, 0, 0),
    (255, 51, 0),
    (2255, 255, 0),
    (20, 20, 20),
]

byte_stream = []

for i in range(im.size[1]):  # y
    for j in range(im.size[0]):  # x
        rgb = pix[j, i][:3]  # Get RGB (ignore alpha if present)
        if rgb not in Color_Plate:
            index = find_closest_color(rgb, Color_Plate)
        else:
            index = Color_Plate.index(rgb)
        byte_stream.append(index)

print("Color Plate Info:")
for i, color in enumerate(Color_Plate):
    print(f"{i}: {color}")

print("Coded File Length (Bytes): ", len(byte_stream))

saved_file_name = input("Enter Saved Filename: ")
with open(saved_file_name, 'w') as f:
    write_stream = '\n'.join(f'{a:01x}' for a in byte_stream)
    f.write(write_stream)
