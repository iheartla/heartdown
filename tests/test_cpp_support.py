"""
Check app.copy_cpp_support(), which copies the C++ support files that a paper's lib.h includes into
output_code/heartlang/: type_helper.h for derivatives, and the I❤️MESH classes, with the headers they include (in
include/) and their source files (in src/).
"""
import pytest

from heartdown import app


@pytest.mark.parametrize("includes, files", [
    ('#include <Eigen/Core>\n', set()),
    ('#include "type_helper.h"\n', {"type_helper.h"}),
    ('#include "type_helper.h"\n#include "FaceMesh.h"\n#include "MeshHelper.h"\n',
     {"type_helper.h", "FaceMesh.h", "MeshHelper.h", "MeshHelper.cpp", "TriangleMesh.h", "TriangleMesh.cpp",
      "Connectivity.h", "Connectivity.cpp", "dec_util.h", "dec_util.cpp"}),
    ('#include "PointCloud.h"\n',
     {"PointCloud.h", "PointCloud.cpp", "nanoflann.hpp", "Connectivity.h", "Connectivity.cpp",
      "dec_util.h", "dec_util.cpp"}),
])
def test_copy_cpp_support(tmp_path, includes, files):
    app.copy_cpp_support(includes + "struct paper {};\n", tmp_path)
    copied = {p.relative_to(tmp_path).as_posix() for p in tmp_path.rglob("*") if p.is_file()}
    assert copied == {"heartlang/{}/{}".format("src" if f.endswith(".cpp") else "include", f) for f in files}
