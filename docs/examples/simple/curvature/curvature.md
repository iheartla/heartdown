---
full_paper: False
---
# Gaussian Curvature
❤: curvature

A smooth surface's Gaussian curvature measures how much it bends at each point, in two directions at once. On <span class="def">a triangle mesh $M$</span> with <span class="def">a position $x_i$ for each vertex $i$</span>, curvature lives at the vertices: around a flat vertex, the angles of its faces add up to $2π$, and the angle defect, how much less they add up to, is the curvature concentrated there. I❤️MESH gives us <span class="def">the vertices $V$</span>, <span class="def">edges $E$</span> and <span class="def">faces $F$</span> of the mesh, and the neighborhoods of its elements, such as <span class="def">the faces around a vertex, $\mathit{Faces}$</span>:

```iheartla
ElementSets from MeshConnectivity
Faces, OrientedVertices, NeighborVerticesInFace from Neighborhoods(M)
arccos from trigonometry

V, E, F = ElementSets(M)
where
M ∈ FaceMesh
x_i ∈ ℝ³
```

The <span class="def">angle $θ$ of a face $f$ at its vertex $i$</span> is between the edges to its other two vertices $j$ and $k$:

```iheartla
θ(i, f) = arccos((x_j - x_i)⋅(x_k - x_i)/(‖x_j - x_i‖ ‖x_k - x_i‖)) where i ∈ V, f ∈ F,
j, k = NeighborVerticesInFace(f, i)
```

and the <span class="def">angle defect $K$ at vertex $v$</span> sums the angles of the faces around it:

```iheartla
K(v) = 2π - ∑_(f ∈ Faces(v)) θ(v, f) where v ∈ V
```

The angle defect is curvature times area, so we divide it by a third of <span class="def">the area $A$ of each face $f$</span> around the vertex to plot the curvature:

```iheartla
A(f) = ½‖(x_j - x_i)×(x_k - x_i)‖ where f ∈ F,
i, j, k = OrientedVertices(f)
```

By the Gauss–Bonnet theorem, the angle defects of a closed surface add up to $2π$ times its Euler characteristic, however it bends: $4π$ for a sphere and $0$ for a torus, whose positive curvature on the outside cancels its negative curvature on the inside.

<figure>
```python
import numpy as np
from lib import *

# A torus with radii R and r, whose vertices are a grid of angles u around its axis and w around its tube
R, r, nu, nw = 1, .4, 40, 20
u, w = np.meshgrid(2 * np.pi * np.arange(nu) / nu, 2 * np.pi * np.arange(nw) / nw, indexing='ij')
x = np.stack([(R + r * np.cos(w)) * np.cos(u), (R + r * np.cos(w)) * np.sin(u), r * np.sin(w)], -1).reshape(-1, 3)
# two triangles for each square of the grid, which wraps around in both directions
i, j = np.meshgrid(np.arange(nu), np.arange(nw), indexing='ij')
a, b = i * nw + j, (i + 1) % nu * nw + j
c, d = (i + 1) % nu * nw + (j + 1) % nw, i * nw + (j + 1) % nw
F = np.concatenate([np.stack([a, b, c], -1).reshape(-1, 3), np.stack([a, c, d], -1).reshape(-1, 3)])

surface = curvature(TriangleMesh(F), x)
K = np.array([surface.K(v) for v in range(len(x))])
area = np.zeros(len(x))
for f in range(len(F)):
    area[F[f]] += surface.A(f) / 3

import plotly.graph_objects as go
fig = go.Figure(go.Mesh3d(x=x[:, 0], y=x[:, 1], z=x[:, 2], i=F[:, 0], j=F[:, 1], k=F[:, 2],
                          intensity=K / area, colorscale='RdBu', cmid=0, colorbar=dict(title='K / area'),
                          lighting=dict(ambient=.5, diffuse=.7, specular=.2)))
fig.update_scenes(aspectmode='data', camera=dict(eye=dict(x=1.1, y=1.1, z=1.1)),
                  xaxis_visible=False, yaxis_visible=False, zaxis_visible=False)
fig.update_layout(margin=dict(r=0, l=0, b=0, t=40),
                  title=dict(text='Total angle defect: {:.1e}'.format(K.sum()), x=.5))
fig.write_html('torus.html')
```
<img src="torus.html" alt="The Gaussian curvature of a torus">
<figcaption>The Gaussian curvature of a torus: positive (blue) on the outside, negative (red) on the inside. Its angle defects add up to $0$.</figcaption>
</figure>
