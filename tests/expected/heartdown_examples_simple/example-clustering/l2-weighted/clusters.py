from lib import *
import plotly.express as px
import numpy as np
np.random.seed(0)

# Random 2D data
# x_i = np.random.random( ( 100, 2 ) ) * 5 - 2.5
x_i = np.random.randn( 100, 2 )
x_i[-1] = ( +9, +9.5 )
x_i[-2] = ( +8, -9 )
x_i[-3] = ( -9.5, -9.6 )
x_i[-4] = ( -9, +9 )

# Weight based on distance from the origin.
w_i = np.ones( 100 )
w_i[-4:] = .5

# Initial cluster centers
k = 4
c_i = np.random.randn( 4, 2 )

iterations = 0
while True:
    iterations += 1

    # All distances give us labels
    d_ij = np.sqrt( ( ( x_i[...,None] - c_i.T[None,...] )**2 ).sum( axis = 1 ) )
    labels = d_ij.argmin( axis = 1 )

    # Update c_i with the minimization algorithm
    c_ip = np.asarray([ clustering( x_i[ labels == i ], w_i[ labels == i ] ).c for i in range(4) ])

    if np.allclose( c_ip, c_i ) or iterations > 100: break

    c_i = c_ip.copy()

fig = px.scatter( x = x_i[:, 0], y = x_i[:, 1], color = labels.astype('str') )
fig.add_scatter( x = c_i[:, 0], y = c_i[:, 1], mode="markers", marker=dict(size=10, color="black"))
fig.update_xaxes(range=[-11, 11])
fig.update_yaxes(range=[-11, 11])
fig.update_layout(showlegend=False)
fig.write_html( 'clusters.html' )
