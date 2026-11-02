from lib import *
import numpy as np
import skimage.io
import scipy.signal

# Initialize our filter function
r = 5
f = filters().f

# Store the function into a filter kernel
filter = np.zeros((2*r+1, 2*r+1))
for i in range( 2*r+1 ):
    for j in range( 2*r + 1 ):
        filter[ i,j ] = f( (i-r)/r ) * f( (j-r)/r )

# Normalize the filter.
filter /= filter.sum()

img = skimage.io.imread( "wikipedia.png" )

output = np.stack( [ scipy.signal.convolve2d( img[:,:,c], filter, mode='same', boundary='symm' ) for c in range( img.shape[2] ) ], axis=2)

skimage.io.imsave( "output.png", output )
