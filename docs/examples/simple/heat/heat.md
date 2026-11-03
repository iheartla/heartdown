---
full_paper: False
---
# Heat Diffusion
❤: heat

Heat spreads over a surface according to the heat equation. We discretize the surface as <span class="def">a triangle mesh with $n$ vertices</span>, with <span class="def">a positive semi-definite cotangent Laplacian matrix $L$</span> and <span class="def">a diagonal mass matrix $M$ of the areas around each vertex</span>. One implicit Euler step of <span class="def">duration $t$</span> diffuses <span class="def">an initial heat $δ$ at each vertex</span> to <span class="def">the heat $u$</span>:

```iheartla
u = (M + t L)⁻¹ (M δ)
where
M ∈ ℝ^(n×n) sparse
L ∈ ℝ^(n×n) sparse
δ ∈ ℝ^n
t ∈ ℝ
```

The figure builds the mesh and both matrices with [libigl](https://libigl.github.io/): it subdivides an icosahedron, pushes the result onto a bumpy sphere, and computes $L$ with `igl.cotmatrix` and $M$ with `igl.massmatrix`.

<figure>
```python
import igl
import numpy as np
from lib import *

# A sphere: an icosahedron, Loop-subdivided 4 times and projected onto the unit sphere
V, F = igl.icosahedron()
for _ in range(4):
    V, F = igl.loop(V, F)
V /= np.linalg.norm(V, axis=1)[:, None]
# with bumps
V *= 1 + .15 * (np.sin(5 * V[:, [0]]) * np.sin(5 * V[:, [1]]) * np.sin(5 * V[:, [2]]))

L = -igl.cotmatrix(V, F)
M = igl.massmatrix(V, F, igl.MASSMATRIX_TYPE_VORONOI)

# The heat starts in a small disk around the highest vertex
top = V[np.argmax(V[:, 2])]
δ = (np.linalg.norm(V - top, axis=1) < .15).astype(float)

import plotly.graph_objects as go
from plotly.subplots import make_subplots
times = (.01, .1, 1)
fig = make_subplots(rows=len(times), cols=1, specs=[[{'type': 'surface'}]] * len(times),
                    subplot_titles=['t = {}'.format(t) for t in times], vertical_spacing=.04)
for row, t in enumerate(times, 1):
    u = heat(M, L, δ, t).u
    fig.add_trace(go.Mesh3d(x=V[:, 0], y=V[:, 1], z=V[:, 2], i=F[:, 0], j=F[:, 1], k=F[:, 2],
                            intensity=u / u.max(), colorscale='Inferno', cmin=0, cmax=1,
                            lighting=dict(ambient=.5, diffuse=.7, specular=.2), showscale=row == 1), row, 1)
camera = dict(eye=dict(x=.6, y=.6, z=1.1))
fig.update_scenes(aspectmode='data', camera=camera,
                  xaxis_visible=False, yaxis_visible=False, zaxis_visible=False)
fig.update_layout(margin=dict(r=0, l=0, b=10, t=30))
fig.write_html('diffusion.html')
```
<img src="diffusion.html" alt="Heat spreading from the top of a bumpy sphere">
<figcaption>Heat diffusing from a disk at the top of a bumpy sphere for increasing durations $t$, divided by its maximum.</figcaption>
</figure>
