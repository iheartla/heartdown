from lib import *
import make_cylinder

# Load cylinder with n vertices
mesh = make_cylinder.make_cylinder( 10, 10 )
make_cylinder.save_obj( mesh, 'input.obj', clobber = True )
V = mesh.v
F = mesh.fv
n = len(V)

# Extract the mesh edges
edges = set()
for face in F:
    for fvi in range(3):
        vi,vj = face[fvi], face[(fvi+1)%3]
        edges.add( ( min(vi,vj), max(vi,vj) ) )

# The constraint vector is all vertices with z < 1/4 or z > 3/4
B = np.zeros( n, dtype = int )
B[ V[:,2] < 1/4 ] = 1
B[ V[:,2] > 3/4 ] = 1

# Rotate the top around the z axis by 90 degrees.
R = np.array([[ 1, 0, 0 ],
              [ 0, 0, 1 ],
              [ 0, -1, 0 ]])
for vi in np.where(V[:,2] > 3/4)[0]: V[vi] = R @ V[vi] + (0,1,2)

# Solve for new positions
result = fairing( E = edges, n = n, B = B, V = V )
mesh.v = result.V_apostrophe
make_cylinder.save_obj( mesh, 'solved.obj', clobber = True )

import plotly.graph_objects as go
fig = go.Figure(data=[go.Mesh3d(
    x=mesh.v[:,0], y=mesh.v[:,1], z=mesh.v[:,2],
    i=mesh.fv[:,0], j=mesh.fv[:,1], k=mesh.fv[:,2]
    )])
fig.update_layout( scene_camera={'eye':dict(x=2.5,y=0,z=0), 'up':dict(x=0,y=0,z=1)}, margin=dict(t=0, r=0, l=0, b=0) )
fig.write_html( 'cylinder.html' )
