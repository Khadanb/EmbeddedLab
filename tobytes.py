from PIL import Image

file_name = input("Enter File Name: ")

im = Image.open(file_name)

pix = im.load()

print("Image Size: ")
print(im.size)

x,y = im.size
print(im.mode)

pix_index_list = []

Color_Plate = [('0x92', '0x90', '0xff')]
byte_stream = []
byte_write = 0x00

for i in range(y):
    for j in range(x):
        R = hex(pix[j,i][0])
        G = hex(pix[j,i][1])
        B = hex(pix[j,i][2])
        RGB = (R, G, B)
        if RGB not in Color_Plate:
            Color_Plate.append(RGB)
        ind = Color_Plate.index(RGB)
        pix_index_list.append(ind)
        byte_write = ind
        byte_stream.append(byte_write)

print("Color Plate Info")

for i in range(len(Color_Plate)):
    print(f"{i}: {Color_Plate[i]}")

print("Coded File Length (Bytes): ")
print(len(byte_stream))
saved_file_name = input("Enter Saved Filename: ")
s_f = open(saved_file_name, 'w')
write_stream = '\n'.join(f'{a:01x}' for a in byte_stream)
s_f.write(write_stream)
s_f.close()

