import netCDF4
import numpy as np
from scipy.signal import convolve2d
from matplotlib import pyplot as plt


# open a local NetCDF file or remote OPeNDAP URL
#url = '../../OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc'
#url = '../../output/conv_parallel_v1.nc'
url = 'parallel2.nc'
nc = netCDF4.Dataset(url)

# examine the variables
print nc.variables.keys()
print nc.variables['CMI']

# sample every 10th point of the 'z' variable
topo = nc.variables['CMI'][::10,::10]

# make image
plt.figure(figsize=(100,100))
plt.gray()
plt.imshow(topo,origin='lower')
#plt.imshow(topo, cmap='gray', vmin=0, vmax=4096)
plt.title(nc.title)
plt.savefig('../../output/outputpar_omp_for.png', bbox_inches=0)
print "FINISH"

