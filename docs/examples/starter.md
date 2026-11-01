---
title: A H❤️rtDown Paper
full_paper: False
---
❤: example

# Filtering

To filter a signal, we take a weighted average of the values in a window around each point.
The weights come from a <span class="def">filter function $f$</span> of <span class="def">an offset $x$ in $[-1,1]$</span>:

```iheartla
f(x) = { 1-|x| if |x| < 1
         0 otherwise where x ∈ ℝ
```

Click the equation to edit its I❤️LA code, or the figure below to edit the Python code that draws it with the compiled $f$.

<figure>
```python
from lib import *
import numpy as np
import matplotlib.pyplot as plt

f = example().f
x = np.linspace(-1.5, 1.5, 301)
plt.figure(figsize=(6, 2.5))
plt.plot(x, [f(v) for v in x])
plt.xlabel("x")
plt.ylabel("f(x)")
plt.tight_layout()
plt.savefig("plot.png", dpi=150)
```
<img src="plot.png" alt="The filter function" style="max-width: 100%">
<figcaption>The filter function $f$.</figcaption>
</figure>
