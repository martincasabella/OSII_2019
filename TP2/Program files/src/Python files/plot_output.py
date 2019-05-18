import numpy as np
from netCDF4 import Dataset
from scipy.signal import convolve2d, fftconvolve
from matplotlib import pyplot as plt

NX =  21696
PNX = NX - 3 + 1

'''Asi queda la original y la otra en el mismo grafico'''

# fig, axes = plt.subplots(nrows=1, ncols=2, sharey=True, sharex=True)
# plt.title('Procedural Output')
# matrix = np.fromfile("org_img.bin", dtype=np.float32)
# matrix = matrix.reshape(NX,NX)
# axes[0].imshow(matrix, cmap='gray', vmin=0, vmax=800)
# matrix = np.fromfile("procedural_out.bin", dtype=np.float32)
# matrix = matrix.reshape(PNX,PNX)
# axes[1].imshow(matrix, cmap='gray', vmin=0, vmax=800)
# fig.savefig('image_results_procedural.svg', format='svg', dpi=800)
# #plt.imshow(matrix, cmap='gray')
# plt.show()

fig, axes = plt.subplots(nrows=1, ncols=2, sharey=True, sharex=True)
plt.title('Parallel Output')
matrix = np.fromfile("org_img.bin", dtype=np.float32)
matrix = matrix.reshape(NX,NX)
axes[0].imshow(matrix, cmap='gray', vmin=0, vmax=800)
matrix = np.fromfile("parallel_out1.bin", dtype=np.float32)
matrix = matrix.reshape(PNX,PNX)
axes[1].imshow(matrix, cmap='gray', vmin=0, vmax=800)
fig.savefig('../../output/parallel1.svg', format='svg', dpi=800)
#plt.imshow(matrix, cmap='gray')
plt.show()

