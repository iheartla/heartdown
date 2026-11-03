"""
Check app.paper_requirements(), which reads the PyPI requirements that a paper's YAML front matter lists for its
figures. The editor installs them; test_browser.py checks that.
"""
import pytest

from heartdown import app


@pytest.mark.parametrize("paper, requirements", [
    ("---\nfull_paper: False\nrequirements:\n  - libigl>=2.6.4.dev3\n  - plotly\n---\n# Title\n",
     ["libigl>=2.6.4.dev3", "plotly"]),
    ("---\nrequirements: plotly\n---\n# Title\n", ["plotly"]),
    ("---\nrequirements:\n  - plotly\n...\n# Title\n", ["plotly"]),
    ("---\nfull_paper: False\n---\n# Title\n", []),
    ("# Title\n\nrequirements: plotly\n", []),
    ("---\nrequirements:\n---\n# Title\n", []),
    ("---\nrequirements: {plotly: 6}\n---\n# Title\n", []),
    ("---\n: : bad\n---\n# Title\n", []),
])
def test_paper_requirements(paper, requirements):
    assert app.paper_requirements(paper) == requirements
