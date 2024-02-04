# Used to create a random PPM file with different resolutions to test the performance of the implementataions
# and to create graphs for the Ausarbeitung
import numpy as np

width = 30000
height = 3000

pixels = np.random.randint(0, 256, (height, width, 3), dtype=np.uint8)

header = f'P6\n{width} {height}\n255\n'

filename = f'random{width}by{height}.ppm'

with open(filename, 'wb') as f:
    f.write(header.encode('ascii'))
    pixels.tofile(f)
